#include "opendlms/transport/tcp_wrapper.hpp"

namespace opendlms {

    std::vector<uint8_t> TcpWrapper::wrap(
        uint16_t sourceWport, 
        uint16_t destWport, 
        const std::vector<uint8_t>& apdu) 
    {
        std::vector<uint8_t> packet;
        packet.push_back(0x00); // Version High
        packet.push_back(0x01); // Version Low
        
        packet.push_back(static_cast<uint8_t>(sourceWport >> 8));
        packet.push_back(static_cast<uint8_t>(sourceWport & 0xFF));
        
        packet.push_back(static_cast<uint8_t>(destWport >> 8));
        packet.push_back(static_cast<uint8_t>(destWport & 0xFF));
        
        uint16_t len = static_cast<uint16_t>(apdu.size());
        packet.push_back(static_cast<uint8_t>(len >> 8));
        packet.push_back(static_cast<uint8_t>(len & 0xFF));
        
        packet.insert(packet.end(), apdu.begin(), apdu.end());
        return packet;
    }

    std::vector<uint8_t> TcpWrapper::unwrap(const std::vector<uint8_t>& wpdu) {
        // Basic sanity check: Header is 8 bytes
        if (wpdu.size() < 8) return {};

        // Verify Version is 0x0001
        if (wpdu[0] != 0x00 || wpdu[1] != 0x01) return {};

        // Extract length (Bytes 6 & 7)
        uint16_t expectedLen = (static_cast<uint16_t>(wpdu[6]) << 8) | wpdu[7];

        // Verify we have the full payload
        if (wpdu.size() < static_cast<size_t>(8 + expectedLen)) return {};

        // Extract and return only the APDU
        return std::vector<uint8_t>(wpdu.begin() + 8, wpdu.begin() + 8 + expectedLen);
    }

} // namespace opendlms