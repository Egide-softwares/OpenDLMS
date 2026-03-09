#include "opendlms/common/utils.hpp"
#include <cstdio>
#include <iostream>
#include <iomanip>

namespace opendlms {
    namespace utils {

        // Helper function to display hex bytes nicely
        void printHex(const std::string& label, const std::vector<uint8_t>& data) {
            std::cout << std::left << std::setw(25) << label << ": ";
            
            for (auto b : data) {
                // We use a temporary stringstream to format each byte individually
                // This prevents "sticky" formatting issues
                std::printf("%02x ", b); 
            }
            std::cout << std::endl;
        }

    } // namespace utils
} // namespace opendlms