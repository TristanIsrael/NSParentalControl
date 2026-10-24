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

#include "history.h"
#include "../helpers.h"
#include <string>

/*! @brief Constructs a HistoryEntry with the given parameters.
 * @param uid The account UID.
 * @param date The date string.
 * @param titleId The title ID of the application.
 * @param durationInMinutes The play duration in minutes. */
HistoryEntry::HistoryEntry(AccountUid uid, std::string date, u64 titleId, u16 durationInMinutes) 
: d_(std::make_shared<HistoryEntryData>()) { 
    d_->uid = uid;
    d_->date = date;
    d_->titleId = titleId;
    d_->durationInMinutes = durationInMinutes;
}

/*! @brief Copy constructor for HistoryEntry.
 * @param orig The HistoryEntry to copy. */
HistoryEntry::HistoryEntry(const HistoryEntry& orig)
: d_(orig.d_) {}

/*! @brief Copy assignment operator for HistoryEntry.
 * @param orig The HistoryEntry to assign from.
 * @return Reference to this HistoryEntry. */
HistoryEntry& HistoryEntry::operator=(const HistoryEntry& orig) {    
    d_ = orig.d_;
    return *this;
}

/*! @brief Returns the account UID of this entry.
 * @return The account UID. */
AccountUid HistoryEntry::uid() const {
    return d_->uid;
}

/*! @brief Returns the account UID as a string representation.
 * @return String representation of the UID. */
std::string HistoryEntry::uidAsString() const {
    return alefbet::pctrl::helpers::accountUidToString(d_->uid);
}

/*! @brief Returns the date of this entry.
 * @return The date string. */
std::string HistoryEntry::date() const {
    return d_->date;
}
    
/*! @brief Returns the title ID of this entry.
 * @return The title ID. */
u64 HistoryEntry::titleId() const {
    return d_->titleId;
}

/*! @brief Returns the play duration in minutes.
 * @return Duration in minutes. */
u16 HistoryEntry::durationInMinutes() const {
    return d_->durationInMinutes;
}

/*! @brief Default constructor for History. */
History::History()
: d_(std::make_shared<HistoryData>()) {}

/*! @brief Copy constructor for History.
 * @param orig The History to copy. */
History::History(const History& orig)
: d_(orig.d_) {}

/*! @brief Copy assignment operator for History.
 * @param orig The History to assign from.
 * @return Reference to this History. */
History& History::operator=(const History& orig) {
    d_ = orig.d_;
    return *this;
}

/*! @brief Adds a history entry to the history list.
 * @param entry The entry to add. */
void History::addEntry(const HistoryEntry& entry) {
    d_->history.push_back(entry);
}

/*! @brief Updates an existing history entry or creates a new one if not found.
 * @param uid The account UID to match.
 * @param date The date string to match.
 * @param titleId The title ID to match.
 * @param durationInMinutes The duration to add to the existing entry. */
void History::updateEntry(const AccountUid& uid, const std::string& date, u64 titleId, u16 durationInMinutes) {
    for(auto& existingEntry : d_->history) {
        if(existingEntry.uid().uid[0] == uid.uid[0] &&
           existingEntry.uid().uid[1] == uid.uid[1] &&
           existingEntry.date() == date &&
           existingEntry.titleId() == titleId) {
            existingEntry.setDurationInMinutes(durationInMinutes + existingEntry.durationInMinutes());
            return;
        }
    }
}

/*! @brief Returns all history entries.
 * @return Vector of all history entries. */
std::vector<HistoryEntry> History::entries() const {
    return d_->history;
}

/*! @brief Returns history entries filtered by UID, date, and optionally title ID.
 * @param uid The account UID to filter by.
 * @param date The date string to filter by.
 * @param titleId The title ID to filter by (0 to skip).
 * @return Vector of matching history entries. */
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

/*! @brief Checks if this history entry is valid.
 * @return true if titleId is non-zero and date is not empty. */
bool HistoryEntry::isValid() const {
    return d_->titleId != 0 && !d_->date.empty();
}

/*! @brief Sets the play duration in minutes.
 * @param duration The duration in minutes. */
void HistoryEntry::setDurationInMinutes(u16 duration) {
    d_->durationInMinutes = duration;
}