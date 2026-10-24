/*
 * NS Parental Control
 * Copyright (C) 2025 Tristan Israël
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <sys/stat.h>
#include <switch.h>
#include <mutex>
#include <cstring>
#include <vector>
#include <bit>
#include <ranges>
#include "settings_db.h"
#include "../logger.h"
#include "../helpers.h"

using namespace alefbet::pctrl::logger;
using namespace alefbet::pctrl::helpers;

static const char* DATA_DIR = "/config/parental_control";
static const char* SETTINGS_FILENAME = "/config/parental_control/settings.bin";

namespace alefbet::pctrl::database::settings {

    // Binary format constants (see docs/database-binary-format.md for full spec)
    static constexpr u32 SETTINGS_MAGIC = 0x4E535053; // "NSPS"
    static constexpr u32 SETTINGS_VERSION = 1;

    struct BinHeader {
        u32 magic;
        u32 version;
        u64 entryCount;
    } __attribute__((packed));

    struct BinEntryHeader {
        u32 type;
        u32 keyLength;
    } __attribute__((packed));

    static FsFileSystem sdmc_;
    static FsFile handle_settings_;
    static std::mutex mutex_settings_;
    static bool ready_ = false;
    static bool settings_synchronized_ = false;
    static Settings settings_;

    /*! @brief Initializes the SD card filesystem access.
     * @return true if successful, false otherwise. */
    bool prepare() {
        if(ready_) return true;

        ready_ = R_SUCCEEDED(fsOpenSdCardFileSystem(&sdmc_));
        if(!ready_) {
            logError("[Database] Could not get access to SD card\n");
        }

        return ready_;
    }

    /*! @brief Closes the SD card filesystem handle. */
    void freeFs() {
        if(!ready_) return;

        fsFsClose(&sdmc_);
    }

    /*! @brief Creates the data directory on the SD card if it doesn't exist.
     * @return true if successful, false otherwise. */
    bool createDataDirectory()
    {
        if(!prepare()) return false;

        bool result = R_SUCCEEDED(fsFsCreateDirectory(&sdmc_, DATA_DIR));

        return result;
    }

    /*! @brief Loads settings entries from a binary buffer in memory.
     * @param data Pointer to the binary data.
     * @param fileSize Size of the binary data in bytes.
     * @return true if parsing succeeded, false otherwise. */
    static bool loadEntriesFromBinary(const u8* data, s64 fileSize) {
        if(fileSize < static_cast<s64>(sizeof(BinHeader))) {
            logError("[Database] Settings file too small for header\n");
            return false;
        }

        const BinHeader* header = reinterpret_cast<const BinHeader*>(data);

        if(header->magic != SETTINGS_MAGIC) {
            logError("[Database] Invalid settings magic: %08X\n", header->magic);
            return false;
        }

        if(header->version != SETTINGS_VERSION) {
            logError("[Database] Unsupported settings version: %u (expected %u)\n",
                     header->version, SETTINGS_VERSION);
            return false;
        }

        s64 offset = sizeof(BinHeader);
        for(u64 i = 0; i < header->entryCount; i++) {
            if(offset + static_cast<s64>(sizeof(BinEntryHeader)) > fileSize) {
                logError("[Database] Entry %llu exceeds file bounds\n", i);
                return false;
            }

            const BinEntryHeader* entryHdr = reinterpret_cast<const BinEntryHeader*>(data + offset);
            offset += sizeof(BinEntryHeader);

            if(offset + entryHdr->keyLength > fileSize) {
                logError("[Database] Key string exceeds file bounds\n");
                return false;
            }

            std::string keyStr(reinterpret_cast<const char*>(data + offset), entryHdr->keyLength);
            offset += entryHdr->keyLength;

            Setting setting;
            setting.key = keyStr;
            setting.type = static_cast<SettingType>(entryHdr->type);

            switch(setting.type) {
                case INTEGER: {
                    if(offset + 8 > fileSize) {
                        logError("[Database] INTEGER value exceeds file bounds\n");
                        return false;
                    }
                    std::memcpy(&setting.int_value, data + offset, 8);
                    offset += 8;
                    break;
                }
                case DOUBLE: {
                    if(offset + 8 > fileSize) {
                        logError("[Database] DOUBLE value exceeds file bounds\n");
                        return false;
                    }
                    u64 bits;
                    std::memcpy(&bits, data + offset, 8);
                    setting.double_value = std::bit_cast<double>(bits);
                    offset += 8;
                    break;
                }
                case STRING: {
                    if(offset + 4 > fileSize) {
                        logError("[Database] STRING length exceeds file bounds\n");
                        return false;
                    }
                    u32 valueLength;
                    std::memcpy(&valueLength, data + offset, 4);
                    offset += 4;

                    if(offset + valueLength > fileSize) {
                        logError("[Database] STRING value exceeds file bounds\n");
                        return false;
                    }
                    setting.string_value = std::string(reinterpret_cast<const char*>(data + offset), valueLength);
                    offset += valueLength;

                    if(offset + 1 > fileSize) {
                        logError("[Database] STRING encrypted flag exceeds file bounds\n");
                        return false;
                    }
                    u8 encryptedFlag;
                    std::memcpy(&encryptedFlag, data + offset, 1);
                    setting.encrypted = encryptedFlag != 0;
                    offset += 1;
                    break;
                }
            }

            settings_[setting.key] = setting;
        }

        return true;
    }

    /*! @brief Loads the settings from the SD card file and returns the settings map.
     * @return Reference to the loaded settings map. */
    Settings& loadSettings() {
        if(settings_synchronized_) return settings_;

        logDebug("[Database] Loading settings at %s\n", SETTINGS_FILENAME);

        if(!prepare()) return settings_;

        bool opened = R_SUCCEEDED(fsFsOpenFile(&sdmc_, SETTINGS_FILENAME, FsOpenMode_Read, &handle_settings_));

        if(!opened) {
            logError("Could not open settings file. Initializing default settings.\n");

            FsDirEntryType type;
            if(R_FAILED(fsFsGetEntryType(&sdmc_, DATA_DIR, &type))) {
                if(!createDataDirectory()) {
                    logError("[Database] The data directory could not be created.\n");
                    return settings_;
                }
            }

            saveSetting(Setting {
                .key = SETTING_ENABLED,
                .type = INTEGER,
                .int_value = 1
            });
        } else {
            s64 fileSize = 0;
            if(R_FAILED(fsFileGetSize(&handle_settings_, &fileSize))) {
                logError("[Database] Could not get settings file size\n");
                fsFileClose(&handle_settings_);
                return settings_;
            } else {
                logDebug("[Database] Settings file size is %i\n", fileSize);
            }

            if(fileSize == 0) {
                logDebug("[Database] Settings file is empty, initializing default settings\n");
                fsFileClose(&handle_settings_);
                saveSetting(Setting {
                    .key = SETTING_ENABLED,
                    .type = INTEGER,
                    .int_value = 1
                });
                return settings_;
            }

            std::vector<u8> buffer(fileSize);
            u64 dataRead = 0;
            if(R_FAILED(fsFileRead(&handle_settings_, 0, buffer.data(), fileSize, FsReadOption_None, &dataRead))) {
                logError("[Database] Could not read the settings file\n");
                fsFileClose(&handle_settings_);
                return settings_;
            }

            fsFileClose(&handle_settings_);

            if(!loadEntriesFromBinary(buffer.data(), fileSize)) {
                logError("[Database] Failed to parse settings entries\n");
                return settings_;
            }

            logInfo("[Database] Loaded %llu settings from file\n", settings_.size());
        }

        settings_synchronized_ = true;
        return settings_;
    }

    /*! @brief Saves a single setting to the settings map and persists to disk.
     * @param setting The setting to save. */
    void saveSetting(Setting setting) {
        settings_[setting.key] = setting;
        saveSettings();
    }

    /*! @brief Persists all settings from the map to the SD card file. */
    void saveSettings() {
        logDebug("[Database] Saving settings\n");
        std::lock_guard<std::mutex> lock(mutex_settings_);

        if(!prepare()) return;

        const auto& values = settings_ | std::views::values;
        u64 entryCount = settings_.size();

        s64 totalSize = sizeof(BinHeader);
        for(const auto& setting : values) {
            totalSize += sizeof(BinEntryHeader);
            totalSize += setting.key.length();

            switch(setting.type) {
                case INTEGER:
                    totalSize += 8;
                    break;
                case DOUBLE:
                    totalSize += 8;
                    break;
                case STRING:
                    totalSize += 4;
                    totalSize += setting.string_value.length();
                    totalSize += 1;
                    break;
            }
        }

        std::vector<u8> buffer(totalSize);
        u8* writePtr = buffer.data();

        BinHeader header;
        header.magic = SETTINGS_MAGIC;
        header.version = SETTINGS_VERSION;
        header.entryCount = entryCount;
        std::memcpy(writePtr, &header, sizeof(BinHeader));
        writePtr += sizeof(BinHeader);

        for(const auto& setting : values) {
            BinEntryHeader entryHdr;
            entryHdr.type = static_cast<u32>(setting.type);
            entryHdr.keyLength = static_cast<u32>(setting.key.length());
            std::memcpy(writePtr, &entryHdr, sizeof(BinEntryHeader));
            writePtr += sizeof(BinEntryHeader);

            std::memcpy(writePtr, setting.key.data(), setting.key.length());
            writePtr += setting.key.length();

            switch(setting.type) {
                case INTEGER: {
                    std::memcpy(writePtr, &setting.int_value, 8);
                    writePtr += 8;
                    break;
                }
                case DOUBLE: {
                    u64 bits = std::bit_cast<u64>(setting.double_value);
                    std::memcpy(writePtr, &bits, 8);
                    writePtr += 8;
                    break;
                }
                case STRING: {
                    u32 valueLength = static_cast<u32>(setting.string_value.length());
                    std::memcpy(writePtr, &valueLength, 4);
                    writePtr += 4;
                    std::memcpy(writePtr, setting.string_value.data(), valueLength);
                    writePtr += valueLength;
                    u8 encryptedFlag = setting.encrypted ? 1 : 0;
                    std::memcpy(writePtr, &encryptedFlag, 1);
                    writePtr += 1;
                    break;
                }
            }
        }

        logDebug("[Database] Writing %llu bytes (%llu settings) to settings file\n", totalSize, entryCount);

        if(R_FAILED(fsFsDeleteFile(&sdmc_, SETTINGS_FILENAME))) {
            logError("[Database] Could not delete the current settings file\n");
        }

        if(R_FAILED(fsFsCreateFile(&sdmc_, SETTINGS_FILENAME, 0, 0))) {
            logError("[Database] Could not create the settings file\n");
            return;
        }

        if(R_FAILED(fsFsOpenFile(&sdmc_, SETTINGS_FILENAME, FsOpenMode_Write | FsOpenMode_Append, &handle_settings_))) {
            logError("[Database] The settings file could not be opened for writing\n");
            return;
        }

        u64 bytesWritten = 0;
        if(R_FAILED(fsFileWrite(&handle_settings_, 0, buffer.data(), totalSize, FsWriteOption_Flush))) {
            logError("[Database] Could not write into settings file\n");
        } else {
            logDebug("[Database] Successfully wrote %llu bytes to settings\n", totalSize);
        }

        fsFileClose(&handle_settings_);
    }

}
