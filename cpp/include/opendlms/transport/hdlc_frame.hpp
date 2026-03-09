#pragma once
#include <vector>
#include <cstdint>

namespace opendlms {

    class HdlcFrame {
        public:
            // SNRM (Set Normal Response Mode) - The "Hello" for HDLC
            static std::vector<uint8_t> createSnrm(uint16_t clientAddr, uint16_t serverAddr);
            
            // Wraps an APDU into an I-Frame (Information Frame)
            static std::vector<uint8_t> wrap(uint16_t clientAddr, uint16_t serverAddr, 
                                        const std::vector<uint8_t>& apdu, uint8_t frameCount);
    };

}