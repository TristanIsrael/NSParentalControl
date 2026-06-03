#include <vector>
#include <string>
#include <cstddef>
#include <cstring>
#include "stream_reader.h"
#include "types.h"

using namespace alefbet::pctrl;

std::string StreamReader::readString() {
    u32 len = readValue<u32>();

    if(data_.size() < offset + len) {
        //logger::logError("Not enough data to decode");
        return "";
    }

    std::string str;
    str.resize(len);
    std::memcpy(str.data(), data_.data() + offset, len);
    offset += len;

    return str;
}

ByteArray StreamReader::readByteArray() {
    ByteArray data;
    u32 len = readValue<u32>();

    if(data_.size() < offset + len) {
        //logger::logError("Not enough data to decode");
        return ByteArray{};
    }

    //std::memcpy(data.data(), data_.data() + offset, len);
    data = ByteArray{data_.end() - len, data_.end()};

    return data;
}
