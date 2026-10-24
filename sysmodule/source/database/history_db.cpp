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
#include <memory>
#include "history_db.h"
#include "history.h"
#include "../logger.h"
#include "../helpers.h"

using namespace alefbet::pctrl::logger;
using namespace alefbet::pctrl::helpers;

static const char* DATA_DIR = "/config/parental_control";
static const char* DB_FILENAME = "/config/parental_control/sessions.bin";

namespace alefbet::pctrl::database::history {

    // Binary format constants and structures (see history_db.h for full spec)
    static constexpr u32 DB_MAGIC = 0x4E535043; // "NSPC"
    static constexpr u32 DB_VERSION = 1;

    struct BinHeader {
        u32 magic;
        u32 version;
        u64 entryCount;
    } __attribute__((packed));

    struct BinEntry {
        u64 uidLow;
        u64 uidHigh;
        u32 dateLength;
        u64 titleId;
        u16 durationInMinutes;
        // date string follows immediately after the struct
    } __attribute__((packed));

    static FsFileSystem sdmc_;
    static FsFile handle_database_;
    static std::mutex mutex_database_;
    static bool ready_ = false;
    static bool data_synchronized_ = false;
    static History history_;
    static bool mustUpgrade_ = false;
    static bool isTampered_ = false;

    /*! @brief Returns whether a database upgrade is needed.
     * @return true if upgrade is needed, false otherwise. */
    bool upgradeNeeded() {
        return mustUpgrade_;
    }

    /*! @brief Returns whether the database has been tampered with.
     * @return true if tampered, false otherwise. */
    bool isTampered() {
        return isTampered_;
    }

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

    /*! @brief Loads history entries from a binary buffer in memory.
     * @param data Pointer to the binary data.
     * @param fileSize Size of the binary data in bytes.
     * @return true if parsing succeeded, false otherwise. */
    static bool loadEntriesFromBinary(const u8* data, s64 fileSize) {
        if(fileSize < static_cast<s64>(sizeof(BinHeader))) {
            logError("[Database] Database file too small for header\n");
            return false;
        }

        const BinHeader* header = reinterpret_cast<const BinHeader*>(data);

        if(header->magic != DB_MAGIC) {
            logError("[Database] Invalid database magic: %08X\n", header->magic);
            return false;
        }

        if(header->version != DB_VERSION) {
            logError("[Database] Unsupported database version: %u (expected %u)\n", 
                     header->version, DB_VERSION);
            return false;
        }

        s64 offset = sizeof(BinHeader);
        for(u64 i = 0; i < header->entryCount; i++) {
            if(offset + static_cast<s64>(sizeof(BinEntry)) > fileSize) {
                logError("[Database] Entry %llu exceeds file bounds\n", i);
                return false;
            }

            const BinEntry* entry = reinterpret_cast<const BinEntry*>(data + offset);
            offset += sizeof(BinEntry);

            if(offset + entry->dateLength > fileSize) {
                logError("[Database] Date string exceeds file bounds\n");
                return false;
            }

            std::string dateStr(reinterpret_cast<const char*>(data + offset), entry->dateLength);
            offset += entry->dateLength;

            AccountUid uid;
            uid.uid[0] = entry->uidLow;
            uid.uid[1] = entry->uidHigh;

            HistoryEntry he(uid, dateStr, entry->titleId, entry->durationInMinutes);
            history_.addEntry(he);
        }

        return true;
    }

    /*! @brief Loads the history database from the SD card file. */
    void loadDatabase()
    {
        if(data_synchronized_) return;

        std::lock_guard<std::mutex> lock(mutex_database_);

        logDebug("[Database] Loading database at %s\n", DB_FILENAME);

        if(!prepare()) return;

        bool opened = R_SUCCEEDED(fsFsOpenFile(&sdmc_, DB_FILENAME, FsOpenMode_Read, &handle_database_));

        if(!opened) {
            logError("Could not open database file. Creating new database.\n");

            FsDirEntryType type;
            if(R_FAILED(fsFsGetEntryType(&sdmc_, DATA_DIR, &type))) {
                if(!createDataDirectory()) {
                    logError("[Database] The data directory could not be created.\n");
                    return;
                }
            }

            if(R_FAILED(fsFsCreateFile(&sdmc_, DB_FILENAME, 0, 0))) {
                logError("[Database] Could not create a new database file.\n");
                return;
            }

            data_synchronized_ = true;
            return;
        }

        s64 fileSize = 0;
        if(R_FAILED(fsFileGetSize(&handle_database_, &fileSize))) {
            logError("[Database] Could not get database file size\n");
            fsFileClose(&handle_database_);
            return;
        } else {
            logDebug("[Database] Database file size is %i\n", fileSize);
        }

        if(fileSize == 0) {
            logDebug("[Database] Database file is empty, initializing new database\n");
            fsFileClose(&handle_database_);
            data_synchronized_ = true;
            return;
        }

        std::vector<u8> buffer(fileSize);
        u64 dataRead = 0;
        if(R_FAILED(fsFileRead(&handle_database_, 0, buffer.data(), fileSize, FsReadOption_None, &dataRead))) {
            logError("[Database] Could not read the database file\n");
            fsFileClose(&handle_database_);
            return;
        }

        fsFileClose(&handle_database_);

        logDebug("[Database] Successfully read %llu bytes from database\n", dataRead);

        if(!loadEntriesFromBinary(buffer.data(), fileSize)) {
            logError("[Database] Failed to parse database entries\n");
            isTampered_ = true;
            return;
        }

        logInfo("[Database] Loaded %llu entries from database\n", history_.entries().size());

        data_synchronized_ = true;
    }

    /*! @brief Saves the history database to the SD card file. */
    void saveDatabase() {
        std::lock_guard<std::mutex> lock(mutex_database_);

        if(!prepare()) return;

        const auto& entries = history_.entries();
        u64 entryCount = entries.size();

        // Calculate required buffer size
        s64 totalSize = sizeof(BinHeader);
        for(const auto& entry : entries) {
            totalSize += sizeof(BinEntry);
            totalSize += entry.date().length();
        }

        std::vector<u8> buffer(totalSize);
        u8* writePtr = buffer.data();

        // Write header
        BinHeader header;
        header.magic = DB_MAGIC;
        header.version = DB_VERSION;
        header.entryCount = entryCount;
        std::memcpy(writePtr, &header, sizeof(BinHeader));
        writePtr += sizeof(BinHeader);

        // Write entries
        for(const auto& entry : entries) {
            BinEntry binEntry;
            binEntry.uidLow = entry.uid().uid[0];
            binEntry.uidHigh = entry.uid().uid[1];
            
            const auto& dateStr = entry.date();
            binEntry.dateLength = static_cast<u32>(dateStr.length());
            binEntry.titleId = entry.titleId();
            binEntry.durationInMinutes = entry.durationInMinutes();

            std::memcpy(writePtr, &binEntry, sizeof(BinEntry));
            writePtr += sizeof(BinEntry);

            std::memcpy(writePtr, dateStr.data(), dateStr.length());
            writePtr += dateStr.length();
        }

        logDebug("[Database] Writing %llu bytes (%llu entries) to database\n", totalSize, entryCount);

        if(R_FAILED(fsFsDeleteFile(&sdmc_, DB_FILENAME))) {
            logError("[Database] Could not delete the current database file\n");
        }

        if(R_FAILED(fsFsCreateFile(&sdmc_, DB_FILENAME, 0, 0))) {
            logError("[Database] Could not create the database file\n");
            return;
        }

        if(R_FAILED(fsFsOpenFile(&sdmc_, DB_FILENAME, FsOpenMode_Write | FsOpenMode_Append, &handle_database_))) {
            logError("[Database] The database file could not be opened for writing\n");
            return;
        }
        
        if(R_FAILED(fsFileWrite(&handle_database_, 0, buffer.data(), totalSize, FsWriteOption_Flush))) {
            logError("[Database] Could not write into database file\n");
        } else {
            logDebug("[Database] Successfully wrote %llu bytes to database\n", totalSize);
        }

        fsFileClose(&handle_database_);
    }

    /*! @brief Retrieves history entries for a specific user and date.
     * @param uid The account UID to query.
     * @param date The date string to filter by.
     * @return Vector of history entries matching the criteria. */
    std::vector<HistoryEntry> getHistory(AccountUid uid, std::string date) {
        loadDatabase();
        return history_.entries(uid, date);
    }

    /*! @brief Adds a new or updates an existing history entry for a user.
     * @param uid The account UID.
     * @param titleId The title ID of the application.
     * @param duration_in_minutes The duration to add in minutes.
     * @return The updated or newly created history entry. */
    HistoryEntry addToHistory(AccountUid uid, u64 titleId, u16 duration_in_minutes)
    {
        HistoryEntry result;

        if(uid.uid[0] == 0 || uid.uid[1] == 0 || titleId == 0) {
            logError("[Database] Wrong parameters\n");
            return result;
        }

        std::string uidToString = accountUidToString(uid);

        const auto& date = today();
        if(date.empty()) {
            return result;
        }

        auto entries = history_.entries(uid, date, titleId);

        if(!entries.empty()) {
            auto entry = entries[0];
            logDebug("[Database] entry found for user %s with title ID %i\n", uidToString.c_str(), titleId);
            entry.setDurationInMinutes(duration_in_minutes + entry.durationInMinutes());
            logDebug("[Database] new duration is %i\n", entry.durationInMinutes());
            result = entry;
        } else {
            logDebug("[Database] no entry found for user %s with title ID %i\n", uidToString.c_str(), titleId);
            HistoryEntry entry = HistoryEntry(uid, date, titleId, duration_in_minutes);
            history_.addEntry(entry);
            result = entry;
        }

        saveDatabase();
        return result;
    }

}
