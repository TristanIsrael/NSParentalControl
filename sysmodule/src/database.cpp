#include <fstream>
#include <sys/stat.h>
#include <vector>
#include "database.h"
#include "switch.h"
#include "structs.h"
#include "json.hpp"

using json = nlohmann::json;

const std::string DB_FILENAME = "/switch/parental_control/sessions.json";
const std::string SETTINGS_FILENAME = "/switch/parental_control/settings.json";

void ensureDataDirectory() {
    struct stat st;
    if (stat("/switch/parental_control", &st) != 0) {
        mkdir("/switch/parental_control", 0777);
    }
}

void loadDatabase(UserSessions& sessions) {    
    std::ifstream in(DB_FILENAME, std::ios::in);
    if (!in) return;

    if (in.is_open()) {
        json j_data;
        in >> j_data;

        //Iterate thru the accounts data and ignore those not matching current account
        for(size_t i = 0 ; i < j_data.size() ; i++) {
            json account = j_data[i];

            std::string uid = account["account_id"].get<std::string>();

        }

    } else {
        printf("Error: Could not open database.\n");
    }

    in.close();
}

void saveDatabase(std::map<AccountUid, UserSession>& sessions) {
    std::ofstream of(DB_FILENAME, std::ios::out);

    std::vector<json> accounts;

    //TODO: separate configuration and sessions

    for (auto &pair : sessions) {
        UserSession &s = pair.second;
        size_t games_count = s.games.size();        

        std::vector<json> games;
        for (auto &gpair : s.games) {
            GameSession &g = gpair.second;
            json game = json::object({});
            game["game_id"] = g.game_id;
            game["game_title"] = g.game_title;
            game["daily_seconds"] = g.daily_seconds;

            games.push_back(game);
        }

        json account = json::object( {
            { "account_id", s.account_id },
            { "last_day", s.last_day },
            { "total_daily_seconds", s.total_daily_seconds },
            { "games", games }
        });   
        
        accounts.push_back(account);
    }

    json j_accounts = json(accounts);
    of << j_accounts.dump(); 

    of.close();
}

Settings loadSettings() {
    std::ifstream ifs(SETTINGS_FILENAME, std::ios::in);    
    Settings settings;

    if(!ifs) {
        printf("Could not open settings file. Initialise default settings.");
        settings[SETTING_DAILY_LIMIT_GAME].int_value = 1*3600; //Default: 1 hour
        settings[SETTING_DAILY_LIMIT_GLOBAL].int_value = 1*3600; //Default: 1 hour
    } else {
        json j_settings;
        ifs >> j_settings;

        if(j_settings.contains(SETTING_DAILY_LIMIT_GAME)) {
            Setting setting;
            setting.key = SETTING_DAILY_LIMIT_GAME;
            setting.int_value = j_settings[SETTING_DAILY_LIMIT_GAME].get<int>();
            settings[SETTING_DAILY_LIMIT_GAME] = setting;
        }

        if(j_settings.contains(SETTING_DAILY_LIMIT_GLOBAL)) {
            Setting setting;
            setting.key = SETTING_DAILY_LIMIT_GLOBAL;
            setting.int_value = j_settings[SETTING_DAILY_LIMIT_GLOBAL].get<int>();
            settings[SETTING_DAILY_LIMIT_GLOBAL] = setting;
        }

    }

    ifs.close();
    return settings;
}

void saveSettings(Settings& settings, Setting& setting) {
    //Update settings internal structure
    settings[setting.key] = setting;

    json j_settings({});

    //Update database file
    std::ofstream ofs(SETTINGS_FILENAME, std::ios::out);    

    for(const auto& [key, value] : settings) {
        switch(value.type) {
            case INTEGER: j_settings[key] = value.int_value; break;
            case DOUBLE: j_settings[key] = value.double_value; break;
            case STRING: j_settings[key] = value.string_value; break;
        }
    }

    ofs << j_settings;
    ofs.close();
}
