#pragma once
#include <vector>
#include <cstdint>

namespace opendlms {
    namespace utils {

        class Crc16 {
            public:
                // Calculates FCS for HDLC frames
                static uint16_t calculate(const uint8_t* data, size_t length);

            private:
                static const uint16_t table[256];
        };

    } // namespace utils
} // namespace opendlms