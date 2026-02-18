#pragma once
#include <vector>
#include <cstdint>
#include "opendlms/common/types.hpp"

namespace opendlms {

    // axdr decoder for DLMS APDU responses
    class AxdrDecoder {
        public:
            /**
             * @brief Decodes a DLMS APDU into a variant DlmsValue.
             * Automatically identifies the tag and parses the corresponding data.
             */
            static DlmsValue decode(const std::vector<uint8_t>& response);

        private:
            static size_t findDataPayloadStart(const std::vector<uint8_t>& data);
    };

} // namespace opendlms