#pragma once
#include <vector>
#include <cstdint>

namespace opendlms {

    class TcpWrapper {
        public:
            // Wraps an APDU into a complete WPDU (Wrapper Protocol Data Unit)
            static std::vector<uint8_t> wrap(
                uint16_t sourceWport, 
                uint16_t destWport, 
                const std::vector<uint8_t>& apdu);

            // Unwraps a received WPDU to extract the APDU
            static std::vector<uint8_t> unwrap(const std::vector<uint8_t>& wpdu);
    };

} // namespace opendlms