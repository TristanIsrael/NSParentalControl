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

#pragma once
#include <memory>
#include <string>
#include <vector>
#include <switch.h>

class HistoryEntryData {
public:    
    AccountUid uid = {0};
    std::string date;
    u64 titleId = 0;
    u16 durationInMinutes = 0;
};

class HistoryEntry {
public:    
    HistoryEntry() {}
    HistoryEntry(AccountUid uid, std::string date, u64 titleId, u16 durationInMinutes);
    HistoryEntry(const HistoryEntry& orig);
    HistoryEntry& operator=(const HistoryEntry& orig);

    bool isValid() const;
    AccountUid uid() const;
    std::string uidAsString() const;
    std::string date() const;
    u64 titleId() const;
    u16 durationInMinutes() const;
    void setDurationInMinutes(u16 duration);

private:
    std::shared_ptr<HistoryEntryData> d_;
};

class HistoryData {
public:
    std::vector<HistoryEntry> history;
};

class History {
public:
    History();
    History(const History&);
    History& operator=(const History&);
    
    void addEntry(const HistoryEntry& entry);
    void updateEntry(const AccountUid& uid, const std::string& date, u64 titleId, u16 durationInMinutes);
    std::vector<HistoryEntry> entries() const;
    std::vector<HistoryEntry> entries(const AccountUid& uid, const std::string& date, const u64& titleId = 0) const;

private:
    std::shared_ptr<HistoryData> d_;    
};
