#pragma once
#include <vector>
#include <cstdint>

namespace opendlms {

    class AarqBuilder {
        public:
            // Builds a standard Public Client AARQ
            static std::vector<uint8_t> buildWrapper();
            static std::vector<uint8_t> buildHdlc();
    };

} // namespace opendlms