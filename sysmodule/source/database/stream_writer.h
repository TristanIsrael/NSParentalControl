#include <vector>
#include <string>
#include <cstddef>
#include <cstring>
#include "byte_array.h"
#include "types.h"
#include "logger.h"

using namespace alefbet::pctrl;

class StreamWriter {
    public:
        StreamWriter(ByteArray& buffer): buffer_(buffer) {}

        template<typename T>
        void appendValue(const T& value) {
            if(!std::is_trivially_copyable_v<T>) {
                logger::logError("The type is not serializable");
                return;
            }
            size_t oldSize = buffer_.size();
            buffer_.resize(oldSize + sizeof(T));
            std::memcpy(buffer_.data() + oldSize, &value, sizeof(T));
        }

        template<typename T>
        T readValue() {}
        void appendString(const std::string& str) {
            u32 len = str.size();
            appendValue(len);

            size_t oldSize = buffer_.size();
            buffer_.resize(oldSize + len);
            std::memcpy(buffer_.data() + oldSize, str.data(), len);
        }

        void appendByteArray(const ByteArray& data);

    private:
        ByteArray& buffer_;
};
