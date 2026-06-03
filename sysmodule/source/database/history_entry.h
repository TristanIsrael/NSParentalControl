#include <string>
#include <vector>
#include <memory>
#include "types.h"
#include "byte_array.h"

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
    bool operator==(const HistoryEntry& orig) const;

    bool isValid() const;
    AccountUid uid() const;
    std::string uidAsString() const;
    std::string date() const;
    u64 titleId() const;
    u16 durationInMinutes() const;
    void setDurationInMinutes(u16 duration);

    ByteArray toByteArray() const;
    static HistoryEntry fromByteArray(const ByteArray& data);

private:
    std::shared_ptr<HistoryEntryData> d_;
};