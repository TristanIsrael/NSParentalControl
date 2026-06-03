#include "history_entry.h"
#include "stream_writer.h"
#include "stream_reader.h"
#ifndef NO_SWITCH_H
#include "helpers.h"
using namespace alefbet::pctrl::helpers;
#endif

/***
 * 
 * HistoryEntry
 */
HistoryEntry::HistoryEntry(AccountUid uid, std::string date, u64 titleId, u16 durationInMinutes) 
: d_(std::make_shared<HistoryEntryData>()) { 
    d_->uid = uid;
    d_->date = date;
    d_->titleId = titleId;
    d_->durationInMinutes = durationInMinutes;
}

bool HistoryEntry::operator==(const HistoryEntry& orig) const {
    return (
        d_->uid.uid[0] == orig.d_->uid.uid[0]
        && d_->uid.uid[1] == orig.d_->uid.uid[1]
        && d_->date == orig.d_->date
        && d_->titleId == orig.d_->titleId
        && d_->durationInMinutes == orig.d_->durationInMinutes
    );
}

HistoryEntry::HistoryEntry(const HistoryEntry& orig)
: d_(orig.d_) {}

HistoryEntry& HistoryEntry::operator=(const HistoryEntry& orig) {    
    d_ = orig.d_;
    return *this;
}

AccountUid HistoryEntry::uid() const {
    return d_->uid;
}

std::string HistoryEntry::uidAsString() const {
#ifndef NO_SWITCH_H
    return accountUidToString(d_->uid);
#else 
    return "";
#endif
}

std::string HistoryEntry::date() const {
    return d_->date;
}
    
u64 HistoryEntry::titleId() const {
    return d_->titleId;
}

u16 HistoryEntry::durationInMinutes() const {
    return d_->durationInMinutes;
}

bool HistoryEntry::isValid() const {
    return d_->titleId != 0 && !d_->date.empty();
}

void HistoryEntry::setDurationInMinutes(u16 duration) {
    d_->durationInMinutes = duration;
}

/*!
    \brief Serialize the HistoryEntry data in a byte array
*/
ByteArray HistoryEntry::toByteArray() const {
    ByteArray data;
    StreamWriter st(data);

    st.appendValue<u64>(d_->uid.uid[0]);
    st.appendValue<u64>(d_->uid.uid[1]);
    st.appendValue<u64>(d_->titleId);
    st.appendValue<u16>(d_->durationInMinutes);
    st.appendString(d_->date);

    return data;
}

HistoryEntry HistoryEntry::fromByteArray(const ByteArray& data) {    
    StreamReader st(data);
    
    AccountUid uid;
    uid.uid[0] = st.readValue<u64>();
    uid.uid[1] = st.readValue<u64>();

    u64 titleId = st.readValue<u64>();
    u16 durationInMinutes = st.readValue<u16>();
    std::string date = st.readString();

    HistoryEntry entry(uid, date, titleId, durationInMinutes);

    return entry;
}
