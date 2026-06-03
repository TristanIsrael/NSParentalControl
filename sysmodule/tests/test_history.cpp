#include <catch2/catch_test_macros.hpp>
#include "database/history_entry.h"
#include "database/stream_reader.h"

TEST_CASE("Serialize history entry", "[HistoryEntry]") {  
    AccountUid uid;
    uid.uid[0] = 0x1111111111;
    uid.uid[1] = 0x2222222222;
    std::string date = "2026/04/01 11:32:12";
    u64 titleId = 0x12715276f7bb765;
    u16 duration = 132;

    HistoryEntry entry(uid, date, titleId, duration);

    // Serialize
    const auto& data = entry.toByteArray();

    // Then deserialize
    HistoryEntry entryTest = HistoryEntry::fromByteArray(data);
    
    // And finally compare both
    REQUIRE(entry == entryTest);
}