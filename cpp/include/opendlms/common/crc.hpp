#pragma once
#include <vector>
#include <cstdint>

namespace opendlms {
    namespace utils {

    class Crc16 {
        public:
            // Calculates FCS for HDLC frames
            static uint16_t calculate(const uint8_t* data, size_t length) {
                uint16_t crc = 0xFFFF;
                for (size_t i = 0; i < length; ++i) {
                    // Look up table using the current byte and the current CRC state
                    crc = (crc >> 8) ^ table[(crc ^ data[i]) & 0xFF];
                }
                return crc ^ 0xFFFF; // Final XOR
            }

        private:
            static const uint16_t table[256];
    };

    } // namespace utils
} // namespace opendlms