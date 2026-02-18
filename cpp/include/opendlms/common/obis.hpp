#pragma once
#include <array>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstdint>

namespace opendlms {
    class ObisCode {
        public:
            std::array<uint8_t, 6> bytes;

            // Default constructor (zeros)
            ObisCode() : bytes{0, 0, 0, 0, 0, 0} {}

            // Constructor from 6 explicit bytes
            ObisCode(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f) 
                : bytes{a, b, c, d, e, f} {}

            // Constructor from string (e.g., "1.0.1.8.0.255")
            explicit ObisCode(const std::string& obisStr) {
                std::stringstream ss(obisStr);
                std::string segment;
                size_t i = 0;
                while (std::getline(ss, segment, '.') && i < 6) {
                    bytes[i++] = static_cast<uint8_t>(std::stoi(segment));
                }
            }

            std::string to_string() const {
                std::stringstream ss;
                for (size_t i = 0; i < 6; ++i) {
                    ss << (int)bytes[i] << (i < 5 ? "." : "");
                }
                return ss.str();
            }
    };

} // namespace opendlms