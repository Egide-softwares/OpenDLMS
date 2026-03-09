#pragma once
#include <vector>
#include <cstdint>

namespace opendlms {

    enum class AssociationResult {
        Accepted = 0,
        Rejected = 1,
        Error = 2
    };

    class AareParser {
        public:
            static AssociationResult parse(const std::vector<uint8_t>& pdu);
    };

} // namespace opendlms