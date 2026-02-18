#pragma once
#include <variant>
#include <vector>
#include <string>
#include <cstdint>
#include "opendlms/common/obis.hpp"

namespace opendlms {

    enum class DlmsTag : uint8_t {
        Null = 0,
        Boolean = 3,
        DoubleLong = 5,        // int32
        DoubleLongUnsigned = 6, // uint32
        OctetString = 9,
        VisibleString = 10,
        Integer = 15,          // int8
        Long = 16,             // int16
        Unsigned = 17,         // uint8
        LongUnsigned = 18      // uint16
    };

    // The central data type of your library
    using DlmsValue = std::variant<
        std::monostate, 
        bool, 
        int8_t, uint8_t, 
        int16_t, uint16_t, 
        int32_t, uint32_t, 
        std::vector<uint8_t>, 
        std::string,
        ObisCode
    >;

} // namespace opendlms