#include "history.h"
#include "../helpers.h"
#include <string>

/*** 
 * 
 * History
 */
History::History()
: d_(std::make_shared<HistoryData>()) {}

History::History(const History& orig)
: d_(orig.d_) {}

History& History::operator=(const History& orig) {
    d_ = orig.d_;
    return *this;
}

void History::addEntry(const HistoryEntry& entry) {
    d_->history.push_back(entry);
}

std::vector<HistoryEntry> History::entries() const {
    return d_->history;
}

std::vector<HistoryEntry> History::entries(const AccountUid& uid, const std::string& date, const u64& titleId) const {
    std::vector<HistoryEntry> entries;

    for(const auto& _entry: d_->history) {
        if(_entry.uid().uid[0] == uid.uid[0] && _entry.uid().uid[1] == uid.uid[1] && _entry.date() == date) {
            if(titleId > 0 && titleId == _entry.titleId()) {
                entries.push_back(_entry);
            } else {
                entries.push_back(_entry);
            }            
        }
    }

    return entries;
}
