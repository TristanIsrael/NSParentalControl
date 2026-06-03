#include <string>
#include "byte_array.h"
#include "logger.h"

using namespace alefbet::pctrl;

class StreamReader {
    public:
        StreamReader(const ByteArray& buf) : data_(buf) {}

        template<typename T>
        T readValue() {
            if(!std::is_trivially_copyable_v<T>) {
                //logger::logError("The type is not copyable");
                return 0;
            }

            if(data_.size() < offset + sizeof(T)) {
                //logger::logError("Not enough data to decode");
                return 0;
            }

            T value;
            std::memcpy(&value, data_.data() + offset, sizeof(T));
            offset += sizeof(T);
            return value;
        }

        std::string readString();

        ByteArray readByteArray();

    private:
        const ByteArray& data_;
        size_t offset = 0;
};
