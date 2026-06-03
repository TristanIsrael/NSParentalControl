#include <vector>
#include <string>
#include <cstddef>
#include <cstring>
#include "stream_writer.h"

void StreamWriter::appendByteArray(const ByteArray& data) {
    u32 len = data.size();
    appendValue(len);

    size_t oldSize = buffer_.size();
    buffer_.resize(oldSize + len);
    std::memcpy(buffer_.data() + oldSize, data.data(), len);
}
