#pragma once
#include <vector>
#include <string>
#include <cstdint>

namespace opendlms {
    namespace utils {

        // Prints a label and a hex dump of the buffer to the console
        void printHex(const std::string& label, const std::vector<uint8_t>& data);

    } // namespace utils
} // namespace opendlms