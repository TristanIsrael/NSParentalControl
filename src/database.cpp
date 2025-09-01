void ensureDataDirectory() {
    struct stat st;
    if (stat("/switch/parental_control", &st) != 0) {
        mkdir("/switch/parental_control", 0777);
    }
}

void loadDatabase() {
    std::ifstream in("/switch/parental_control/sessions.txt", std::ios::binary);
    if (!in) return;

    size_t count;
    in.read((char*)&count, sizeof(count));
    for (size_t i=0; i<count; i++) {
        UserSession s;
        size_t games_count;
        in.read((char*)&s.account_id, sizeof(s.account_id));
        in.read((char*)&s.last_day, sizeof(s.last_day));
        in.read((char*)&s.total_daily_seconds, sizeof(s.total_daily_seconds));
        in.read((char*)&s.total_daily_limit, sizeof(s.total_daily_limit));
        in.read((char*)&games_count, sizeof(games_count));
        for (size_t j=0; j<games_count; j++) {
            GameSession g;
            size_t id_len;
            in.read((char*)&id_len, sizeof(id_len));
            g.game_id.resize(id_len);
            in.read(&g.game_id[0], id_len);
            in.read((char*)&g.daily_seconds, sizeof(g.daily_seconds));
            in.read((char*)&g.daily_limit, sizeof(g.daily_limit));
            s.games[g.game_id] = g;
        }
        sessions[s.account_id] = s;
    }
    in.close();
}

void saveDatabase() {
    std::ofstream out("/switch/parental_control/sessions.txt", std::ios::binary);
    size_t count = sessions.size();
    out.write((char*)&count, sizeof(count));
    for (auto &pair : sessions) {
        UserSession &s = pair.second;
        size_t games_count = s.games.size();
        out.write((char*)&s.account_id, sizeof(s.account_id));
        out.write((char*)&s.last_day, sizeof(s.last_day));
        out.write((char*)&s.total_daily_seconds, sizeof(s.total_daily_seconds));
        out.write((char*)&s.total_daily_limit, sizeof(s.total_daily_limit));
        out.write((char*)&games_count, sizeof(games_count));
        for (auto &gpair : s.games) {
            GameSession &g = gpair.second;
            size_t id_len = g.game_id.size();
            out.write((char*)&id_len, sizeof(id_len));
            out.write(g.game_id.data(), id_len);
            out.write((char*)&g.daily_seconds, sizeof(g.daily_seconds));
            out.write((char*)&g.daily_limit, sizeof(g.daily_limit));
        }
    }
    out.close();
}