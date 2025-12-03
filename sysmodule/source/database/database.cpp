#include <fstream>
#include <sys/stat.h>
#include <vector>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <switch.h>
#include <filesystem>
#include <mutex>
#include <vector>
#include <algorithm>
#include <mbedtls/sha512.h>
#include <iomanip>
#include "database.h"
#include "json.hpp"
#include "../logger.h"
#include "../helpers.h"
//#include "aes.hpp"

using json = nlohmann::json;
using namespace alefbet::pctrl::logger;
using namespace alefbet::pctrl::helpers;

static const char* DATA_DIR = "/config/parental_control";
static const char* DB_FILENAME = "/config/parental_control/sessions.json";
static const char* SETTINGS_FILENAME = "/config/parental_control/settings.json";

namespace alefbet::pctrl::database {
    static FsFileSystem sdmc_;
    static FsFile handle_settings_;
    static std::mutex mutex_settings_;
    static FsFile handle_database_;
    static std::mutex mutex_database_;
    static bool ready_ = false;
    static bool data_synchronized_ = false; // The data are synchronized between the cache and the file
    static bool settings_synchronized_ = false; 
    static History history_;
    static Settings settings_;
    static bool mustUpgrade_ = false;
    static bool isTampered_ = false;

    namespace crypto {
        /*static struct AES_ctx aes_ctx;

        void initCrypto() {
            Result rc = splInitialize();
            if(R_FAILED(rc)) {
                logError("[Database] Could not connect to spl service\n");
                return;
            }

            // Get the device ID            
            u64 deviceId = 0;
            splGetConfig(SplConfigItem_DeviceId, &deviceId);
            if(R_FAILED(rc)) {
                logError("[Database] Failed to get Device ID for ciphering\n");
                return;
            }

            // Derivate a hash
            u8 hash[32] = {0};
            sha256CalculateHash(hash, (u8*)&deviceId, sizeof(deviceId));

            // Create a 128 bit key
            u8 cipher_key[16];
            memcpy(cipher_key, hash, 16);

            // Initialize AES context
            AES_init_ctx(&aes_ctx, cipher_key);            
            
            splExit();
            cipher_ready = true;
        }

        void encryptBuffer(u8* buffer, u64 size) {
            AES_CBC_encrypt_buffer(&aes_ctx, buffer, size);
        }

        void decryptBuffer(u8* buffer, u64 size) {
            AES_CBC_decrypt_buffer(&aes_ctx, buffer, size);
        }*/

        static const char b64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        std::string base64Encode(const std::vector<uint8_t>& data)
        {
            std::string out;
            uint32_t val = 0;
            int valb = -6;

            for (uint8_t c : data) {
                val = (val << 8) + c;
                valb += 8;
                while (valb >= 0) {
                    out.push_back(b64_table[(val >> valb) & 0x3F]);
                    valb -= 6;
                }
            }
            if (valb > -6) out.push_back(b64_table[((val << 8) >> (valb + 8)) & 0x3F]);
            while (out.size() % 4) out.push_back('=');

            return out;
        }

        std::vector<uint8_t> base64Decode(const std::string& s)
        {
            std::vector<int> T(256, -1);
            for (int i = 0; i < 64; i++) T[b64_table[i]] = i;

            std::vector<uint8_t> out;
            uint32_t val = 0;
            int valb = -8;

            for (uint8_t c : s) {
                if (T[c] == -1) break;
                val = (val << 6) + T[c];
                valb += 6;
                if (valb >= 0) {
                    out.push_back((val >> valb) & 0xFF);
                    valb -= 8;
                }
            }
            return out;
        }

        void xorAndPermute(std::vector<uint8_t>& data, uint8_t key) {
            for (auto& b : data)
                b ^= key;

            for (size_t i = 0; i + 3 < data.size(); i += 4) {
                std::swap(data[i],   data[i+2]);
                std::swap(data[i+1], data[i+3]);
            }
        }

        void unpermuteAndXor(std::vector<uint8_t>& data, uint8_t key) {
            for (size_t i = 0; i + 3 < data.size(); i += 4) {
                std::swap(data[i],   data[i+2]);
                std::swap(data[i+1], data[i+3]);
            }

            for (auto& b : data)
                b ^= key;
        }

        std::string encodeValue(const std::string input) {
            std::vector<u8> buf(input.begin(), input.end());
            u8 key = 0x31;

            xorAndPermute(buf, key);
            return std::string(buf.begin(), buf.end());
        }

        std::string decodeValue(const std::string input) {
            std::vector<u8> buf(input.begin(), input.end());
            u8 key = 0x31;

            unpermuteAndXor(buf, key);
            return std::string(buf.begin(), buf.end());
        }

        std::string calculateHash(const json& j_entries) {
            mbedtls_sha512_context ctx;

            const auto& str_data = j_entries.dump();
            u8 hash[64];

            mbedtls_sha512_init(&ctx);
            mbedtls_sha512_starts_ret(&ctx, 0); // 0 = SHA-512
            mbedtls_sha512_update_ret(&ctx, reinterpret_cast<const u8*>(str_data.data()), str_data.size());
            mbedtls_sha512_finish_ret(&ctx, hash);
            mbedtls_sha512_free(&ctx);

            std::ostringstream oss;
            for(int i = 0 ; i < 64 ; i++) {
                oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
            }

            return oss.str();
        }

        bool verifyHash(const json& j_entries) {
            // The hash is calculated without the "hash" entry
            const auto& hash = j_entries["hash"].get<std::string>();
            auto j_copy = j_entries; // Copy JSON data
            j_copy.erase("hash");

            const auto& calculated_hash = calculateHash(j_copy);

            return calculated_hash == hash;
        }
    }

    bool upgradeNeeded() {
        return mustUpgrade_;
    }

    bool isTampered() {
        return isTampered_;
    }

    bool prepare() {
        if(ready_) return true;

        ready_ = R_SUCCEEDED(fsOpenSdCardFileSystem(&sdmc_));
        if(!ready_) {
            logError("[Database] Could not get access to SD card\n");
        }

        return ready_;
    }

    void freeFs() {
        if(!ready_) return;

        fsFsClose(&sdmc_);
    }

    bool createDataDirectory() 
    {
        if(!prepare()) return false;

        bool result = R_SUCCEEDED(fsFsCreateDirectory(&sdmc_, DATA_DIR));

        return result;
    }

    void loadDatabase()
    {   
        // We load the file only when the data are not up to date                 
        if(data_synchronized_) return;

        std::lock_guard<std::mutex> lock(mutex_database_);

        logDebug("[Database] Loading database at %s\n", DB_FILENAME);

        if(!prepare()) return;

        bool opened = R_SUCCEEDED(fsFsOpenFile(&sdmc_, DB_FILENAME, FsOpenMode_Read, &handle_database_));        

        if(!opened) {
            logError("Could not open database file. Try to create a new file.\n");

            // Verify whether data directory exists
            FsDirEntryType type;
            if(R_FAILED(fsFsGetEntryType(&sdmc_, DATA_DIR, &type))) {
                // The directory does not exist
                if(!createDataDirectory()) {
                    logError("[Database] The data directory could not be created.\n");
                    return;
                }
            }

            if(R_FAILED(fsFsCreateFile(&sdmc_, DB_FILENAME, 0, 0))) {
                logError("[Database] Could not create a new database file.\n");
                return;
            }
        } else {
            s64 fileSize = 0;
            if(R_FAILED(fsFileGetSize(&handle_database_, &fileSize))) {
                logError("[Database] Could not get database file size\n");
                fsFileClose(&handle_database_);
                return ;
            } else {
                logDebug("[Database] Database file size is %i\n", fileSize);
            }

            if(fileSize == 0) {
                logError("[Database] Database file is empty\n");
                fsFileClose(&handle_database_);
                return;
            }

            u8* data = new u8[fileSize+1];            
            if(data == nullptr) {
                logError("[Database] Could not create a buffer for sessions\n");
                fsFileClose(&handle_database_);
                return;
            } else {
                logDebug("[Database] sessions buffer ready at @%p\n", (void*)data);
            }

            u64 dataRead = 0;
            if(R_FAILED(fsFileRead(&handle_database_, 0, data, fileSize, FsReadOption_None, &dataRead))) {
                logError("[Database] Could not read the database file\n");
                fsFileClose(&handle_database_);
                return;
            } else {
                logDebug("[Database] Sessions database read\n");
            }

            data[fileSize] = '\0';                

            logDebug("[Database] Sessions data: %s\n", data);
            logDebug("[Database] Parse sessions file\n");
            json j_data = json::parse(data);

            // Verify database integrity
            logInfo("[Database] Verifying database integrity\n");
            if(j_data.contains("hash")) {                                
                isTampered_ = !crypto::verifyHash(j_data);                
                logInfo("[Database] Database file integrity is %s. %s\n", (!isTampered_ ? "verified" : "compromized"), isTampered_ ? "The hash is corrupted." : "");
            } else {
                isTampered_ = true;
                logInfo("[Database] Database file integrity is compromized. The hash is missing\n");
            }            

            std::vector<json> j_entries = j_data["history"].get<std::vector<json>>();            

            for(const auto& j_entry: j_entries) {                
                auto uidAsString = j_entry["uid"].get<std::string>();
                auto date = j_entry["date"].get<std::string>();
                auto titleId = j_entry["title_id"].get<u64>();
                auto durationInMinutes = j_entry["duration_in_min"].get<u16>();
                auto uid = accountUidFromString(uidAsString);

                HistoryEntry entry(uid, date, titleId, durationInMinutes);
                history_.addEntry(entry);
            }

            fsFileClose(&handle_database_);
            delete[] data;

            data_synchronized_ = true;
        }        
    }

    void saveDatabase() {
        std::lock_guard<std::mutex> lock(mutex_database_);

        if(!prepare()) return;

        json j_entries;
        for(const auto& entry: history_.entries()) {
            json j_entry = json::object( {
                { "uid", entry.uidAsString() },
                { "date", entry.date() },
                { "title_id", entry.titleId() },
                { "duration_in_min", entry.durationInMinutes() }
            });

            j_entries.push_back(j_entry);
        }

        json j_history = json{
            { "history", j_entries }
        };

        // Calculate data hash
        const auto& hash = crypto::calculateHash(j_history);

        // Add the hash
        j_history["hash"] = hash;

        if(R_FAILED(fsFsDeleteFile(&sdmc_, DB_FILENAME))) {
            logError("[Database] Could not delete the current database file\n");            
        }

        if(R_FAILED(fsFsCreateFile(&sdmc_, DB_FILENAME, 0, 0))) {
            logError("[Database] Could not create the database file\n");
            return;
        } else {
            logDebug("[Database] New database file created\n");
        }

        if(R_FAILED(fsFsOpenFile(&sdmc_, DB_FILENAME, FsOpenMode_Write | FsOpenMode_Append, &handle_database_))) {
            logError("[Database] The database file could not be opened for writing\n");
            return;
        }

        const auto stdstr_data = j_history.dump();
        const auto str_data = stdstr_data.c_str();
        u64 lenstr = std::strlen(str_data);
        void* s_data = malloc(lenstr+1);
        memset(s_data, 0, lenstr);
        memcpy(s_data, str_data, lenstr);

        logDebug("[Database] Writing sessions data %s (size=%i)\n", s_data, lenstr);    

        if(R_FAILED(fsFileWrite(&handle_database_, 0, s_data, lenstr, FsWriteOption_Flush))) {
            logError("[Database] Could not write into database file\n");
        }

        fsFileClose(&handle_database_);
    }    

    std::vector<HistoryEntry> getHistory(AccountUid uid, std::string date) {
        //Get current history
        loadDatabase();

        return history_.entries(uid, date);
    }

    HistoryEntry addToHistory(AccountUid uid, u64 titleId, u16 duration_in_minutes) 
    {
        HistoryEntry result;

        if(uid.uid[0] == 0 || uid.uid[1] == 0 || titleId == 0) {
            logError("[Database] Wrong parameters\n");
            return result;
        }

        std::string uidToString = accountUidToString(uid);

        //Get current history
        loadDatabase();

        const auto& date = today();
        if(date.empty()) {
            return result;
        }

        //Search existing entry
        auto entries = history_.entries(uid, date, titleId);

        if(!entries.empty()) {
            auto entry = entries[0];
            logDebug("[Database] entry found for user %s with title ID %i\n", uidToString.c_str(), titleId);
            entry.setDurationInMinutes(duration_in_minutes + entry.durationInMinutes());
            logDebug("[Database] new duration is %i\n", entry.durationInMinutes());
            result = entry;
        } else {
            //Or create a new one
            logDebug("[Database] no entry found for user %s with title ID %i\n", uidToString.c_str(), titleId);
            HistoryEntry entry = HistoryEntry(uid, date, titleId, duration_in_minutes);
            history_.addEntry(entry);
            result = entry;
        }

        saveDatabase();
        return result;
    }

    Settings& loadSettings() {
        // We load the file only when the settings are not synchronized
        if(settings_synchronized_) return settings_;

        logDebug("[Database] Loading settings at %s\n", SETTINGS_FILENAME);

        if(!prepare()) return settings_;

        bool opened = R_SUCCEEDED(fsFsOpenFile(&sdmc_, SETTINGS_FILENAME, FsOpenMode_Read, &handle_settings_));

        if(!opened) {
            logError("Could not open settings file. Initialise default settings.\n");

            // Verify whether data directory exists
            FsDirEntryType type;
            if(R_FAILED(fsFsGetEntryType(&sdmc_, DATA_DIR, &type))) {
                // The directory does not exist
                if(!createDataDirectory()) {
                    logError("[Database] The data directory could not be created.\n");
                    return settings_;
                }
            }

            /*saveSetting(Setting {
                .key = SETTING_DAILY_LIMIT_GAME,
                .type = INTEGER,
                .int_value = 1*60 //Default: 1 hour
            });
                        
            saveSetting(Setting {
                .key = SETTING_DAILY_LIMIT_GLOBAL,
                .type = INTEGER,
                .int_value = 1*60 //Default: 1 hour
            });*/
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
                logError("[Database] Settings file is empty\n");
                fsFileClose(&handle_settings_);
                return settings_;
            }

            u8* data_settings = new u8[fileSize+1];            
            if(data_settings == nullptr) {
                logError("[Database] Could not create a buffer for settings\n");
                fsFileClose(&handle_settings_);
                return settings_;
            } else {
                logDebug("[Database] settings buffer ready at @%p\n", (void*)data_settings);
            }

            u64 dataRead = 0;
            if(R_FAILED(fsFileRead(&handle_settings_, 0, data_settings, fileSize, FsReadOption_None, &dataRead))) {
                logError("[Database] Could not read the settings file\n");
                fsFileClose(&handle_settings_);
                return settings_;
            } else {
                logDebug("[Database] Settings database read\n");
            }

            data_settings[fileSize] = '\0';

            logDebug("[Database] Settings data: %s\n", data_settings);
            logDebug("[Database] Parse settings file\n");
            json j_settings = json::parse(data_settings);

            if(j_settings.contains("hash")) {
                logInfo("[Database] Verifying settings integrity\n");
                
                isTampered_ = !crypto::verifyHash(j_settings);

                logInfo("[Database] The settings file integrity is %s\n", (!isTampered_ ? "verified" : "compromized"));
            }

            if(j_settings.contains("settings")) {
                for(const auto& j_setting: j_settings["settings"]) {
                    if(j_setting.is_object() && j_setting.contains("key") && j_setting.contains("type")) {
                        Setting setting;

                        setting.key = j_setting["key"].get<std::string>();
                        setting.type = j_setting["type"].get<SettingType>();
                        
                        switch(setting.type) {
                            case INTEGER: setting.int_value = j_setting["value"].get<u64>(); break;
                            case DOUBLE: setting.double_value = j_setting["value"].get<double>(); break;
                            case STRING: {
                                if(j_setting.contains("encrypted")) {
                                    setting.encrypted = j_setting["encrypted"].get<bool>();                                    
                                }
                                   
                                const auto& val = j_setting["value"].get<std::string>();
                                if(setting.encrypted) {
                                    const auto& decrypted = crypto::decodeValue(val);
                                    setting.string_value = decrypted;
                                } else {
                                    setting.string_value = val;
                                }
                                break;
                            }
                        }

                        settings_[setting.key] = setting;
                    } else {
                        logError("[Database] setting is malformed\n");
                    }
                }
            }

            fsFileClose(&handle_settings_);
            delete[] data_settings;
            settings_synchronized_ = true;
        }
        
        return settings_;
    }

    void saveSetting(Setting setting) {        
        //Update settings internal structure
        settings_[setting.key] = setting;  
        saveSettings();      
    }
    
    void saveSettings() {
        logDebug("[Database] Saving settings\n");
        std::lock_guard<std::mutex> lock(mutex_settings_);        

        if(!prepare()) return;

        //Update database file
        json j_entries;
        
        auto&& values = settings_ | std::views::values;
        for(const auto& value : values) {
            json j_entry = json::object( {
                { "type", value.type },
                { "key", value.key }
            });
            
            std::string valueKey = "value";

            switch(value.type) {
                case INTEGER: j_entry[valueKey] = value.int_value; break;
                case DOUBLE: j_entry[valueKey] = std::to_string(value.double_value); break;
                case STRING: {
                    if(value.encrypted) {
                        const auto& encrypted = crypto::encodeValue(value.string_value);
                        j_entry[valueKey] = encrypted;
                        j_entry["encrypted"] = true;
                    } else {
                        j_entry[valueKey] = value.string_value;
                    }
                    break;
                }
                default: j_entry[valueKey] = "";
            }

            j_entries.push_back(j_entry);
        }        

        json j_settings = json{            
            { "settings", j_entries }
        };

        // Calculate data hash
        const auto& hash = crypto::calculateHash(j_settings);

        // Add the hash
        j_settings["hash"] = hash;
   
        if(R_FAILED(fsFsDeleteFile(&sdmc_, SETTINGS_FILENAME))) {
            logError("[Database] Could not delete the settings file\n"); //Error 514?            
        } else {
            logDebug("[Database] Successfully removed current settings\n");
        }

        if(R_FAILED(fsFsCreateFile(&sdmc_, SETTINGS_FILENAME, 0, 0))) {
            logError("[Database] Could not create the settings file\n");
            return;
        } else {
            logDebug("[Database] New settings file created\n");
        }

        if(R_FAILED(fsFsOpenFile(&sdmc_, SETTINGS_FILENAME, FsOpenMode_Write | FsOpenMode_Append, &handle_settings_))) {
            logError("[Database] The settings file could not be opened for writing\n");
            return;
        } else {
            logDebug("[Database] Settings file successfully opened for writing\n");
        }

        const auto data = j_settings.dump();
        const auto s_data = data.c_str();
        logDebug("[Database] Writing settings %s (size=%i)\n", s_data, std::strlen(s_data));

        if(R_FAILED(fsFileWrite(&handle_settings_, 0, s_data, std::strlen(s_data), FsWriteOption_Flush))) {
            logError("[Database] Could not write into settings file\n");
        }

        fsFileClose(&handle_settings_);
    }

}