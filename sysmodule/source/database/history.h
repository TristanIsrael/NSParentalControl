#pragma once
#include <memory>
#include <string>
#include <vector>
#include "types.h"
#include "history_entry.h"

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
    std::vector<HistoryEntry> entries() const;
    std::vector<HistoryEntry> entries(const AccountUid& uid, const std::string& date, const u64& titleId = 0) const;

private:
    std::shared_ptr<HistoryData> d_;    
};
