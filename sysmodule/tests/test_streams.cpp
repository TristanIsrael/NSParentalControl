#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstddef>
#include "database/byte_array.h"
#include "database/stream_reader.h"
#include "database/stream_writer.h"

TEST_CASE("Serialize ByteArray", "[StreamWriter]") {    
    ByteArray data;
    StreamWriter st(data);
    st.appendValue<u8>(33);
    REQUIRE(data.size() == 1);
    st.appendString("my string");
    REQUIRE(data.size() == 14);
    ByteArray ba;
    ba.push_back(std::byte(10));
    ba.push_back(std::byte(20));
    ba.push_back(std::byte(30));
    ba.push_back(std::byte(40));
    ba.push_back(std::byte(50));
    st.appendByteArray(ba);

    /*std::ostringstream oss;
    for(auto b: data) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(b) << " ";
    }    
    std::cerr << oss.str();*/

    REQUIRE(data.size() == 23);
}

TEST_CASE("Deserialize ByteArray", "[StreamReader]") {
    u8 d[] = {0x21, 0x09, 0x00, 0x00, 0x00, 0x6d, 0x79, 0x20, 0x73, 0x74, 0x72, 0x69, 0x6e, 0x67, 0x05, 0x00, 0x00, 0x00, 0x0a, 0x14, 0x1e, 0x28, 0x32};
    ByteArray data(std::size(d));
    std::transform(std::begin(d), std::end(d), data.begin(), [](int v){ return static_cast<std::byte>(v); });
    StreamReader st(data);

    REQUIRE(data.size() == 23);

    u8 a = st.readValue<u8>();
    std::string str = st.readString();
    ByteArray ba = st.readByteArray();

    REQUIRE(a == 33);
    REQUIRE(str == "my string");
    REQUIRE(ba.size() == 5);
    ByteArray baRef{ std::byte{10}, std::byte{20}, std::byte{30}, std::byte{40}, std::byte{50} };
    REQUIRE(ba == baRef);
}