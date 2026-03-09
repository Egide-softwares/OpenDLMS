#pragma once
#include <vector>
#include <cstdint>
#include "opendlms/common/types.hpp"
#include "opendlms/cosem/cosem_object.hpp"

namespace opendlms {

    class ApduFactory {
        public:
            // Creates a GET-Request-Normal for a COSEM object attribute
            static std::vector<uint8_t> createGetRequest(
                uint8_t invokeId, 
                uint16_t classId,      // Added classId
                const std::string& obisStr, 
                uint8_t attributeId);
    };

} // namespace opendlms