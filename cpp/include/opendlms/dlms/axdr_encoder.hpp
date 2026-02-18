#pragma once
#include <vector>
#include <cstdint>
#include "opendlms/common/types.hpp"

namespace opendlms {

    class AxdrEncoder {
        public:
            // Main entry point: encodes any DlmsValue into the buffer
            static void encode(const DlmsValue& value, std::vector<uint8_t>& buffer);

        private:
            // Specific encoders for different types
            static void writeTag(DlmsTag tag, std::vector<uint8_t>& buffer);
            static void writeUint8(uint8_t val, std::vector<uint8_t>& buffer);
            static void writeUint16(uint16_t val, std::vector<uint8_t>& buffer);
            static void writeUint32(uint32_t val, std::vector<uint8_t>& buffer);
    };

} // namespace opendlms