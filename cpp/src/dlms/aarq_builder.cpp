#include "opendlms/dlms/aarq_builder.hpp"

namespace opendlms {

    std::vector<uint8_t> AarqBuilder::buildDefault() {
        return {
            0x60, 0x1D,             // AARQ Tag, Length 29
            0xA1, 0x09,             // Application Context Name Tag
            0x06, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x01, 0x01, // LN Context
            0xBE, 0x10,             // User Information Tag
            0x04, 0x0E,             // Octet String Tag
            0x01,                   // Initiate Request
            0x00, 0x00, 0x00,       // Unused parameters
            0x06,                   // DLMS Version 6
            0x5F, 0x1F, 0x04,       // Conformance Block Tag
            0x00, 0x08, 0x00,       // <--- Conformance: 524288 (GET only)
            0x03, 0x40,             // Max PDU Size (832 decimal, per XML)
            0x1B                    // VAA Name
        };
    }

} // namespace opendlms