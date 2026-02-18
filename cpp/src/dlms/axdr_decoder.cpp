#include "opendlms/dlms/axdr_decoder.hpp"
#include <stdexcept>

namespace opendlms {

    size_t AxdrDecoder::findDataPayloadStart(const std::vector<uint8_t>& data) {
        for (size_t i = 0; i < data.size() - 3; ++i) {
            // Look for GET-Response-Normal (0xC4 0x01)
            if (data[i] == 0xC4 && data[i+1] == 0x01) {
                // data[i+2] is the Invoke-ID
                // data[i+3] is the Result (0x00 = Success, 0x01 = Data-Access-Result)
                
                if (data[i+3] == 0x01) {
                    // The next byte is the error code (e.g., 0x01 = Object-Undefined)
                    uint8_t errCode = data[i+4];
                    std::string errMsg = "DLMS Error: ";
                    if (errCode == 1) errMsg += "Object-Undefined (Check OBIS)";
                    else if (errCode == 2) errMsg += "Hardware-Fault";
                    else if (errCode == 3) errMsg += "Temporarily-Unavailable";
                    else if (errCode == 4) errMsg += "Read-Write-Denied";
                    throw std::runtime_error(errMsg);
                }
                
                if (data[i+3] == 0x00) return i + 4; // Success! Data tag is here.
            }
        }
        throw std::runtime_error("No valid DLMS response found in frame");
    }

    DlmsValue AxdrDecoder::decode(const std::vector<uint8_t>& response) {
        size_t pos = findDataPayloadStart(response);
        DlmsTag tag = static_cast<DlmsTag>(response[pos]);
        pos++; 

        switch (tag) {
            case DlmsTag::VisibleString: {
                uint8_t len = response[pos++];
                return std::string(response.begin() + pos, response.begin() + pos + len);
            }

            case DlmsTag::OctetString: {
                uint8_t len = response[pos++];
                // Check if this is a Clock (12 bytes)
                if (len == 12) {
                    uint16_t year = (response[pos] << 8) | response[pos + 1];
                    int month = response[pos + 2];
                    int day = response[pos + 3];
                    int hour = response[pos + 5];
                    int min = response[pos + 6];
                    int sec = response[pos + 7];
                    
                    std::stringstream ss;
                    ss << year << "-" << std::setw(2) << std::setfill('0') << month << "-" 
                    << std::setw(2) << std::setfill('0') << day << " "
                    << std::setw(2) << std::setfill('0') << hour << ":" 
                    << std::setw(2) << std::setfill('0') << min << ":" 
                    << std::setw(2) << std::setfill('0') << sec;
                    return ss.str();
                }
                // Otherwise return as raw hex string
                return std::string(response.begin() + pos, response.begin() + pos + len);
            }

            case DlmsTag::LongUnsigned: { // uint16 (Common for Voltage/Energy in some meters)
                uint16_t val = (response[pos] << 8) | response[pos + 1];
                return static_cast<uint32_t>(val); // Cast to uint32 for easier handling in DlmsValue
            }

            case DlmsTag::DoubleLongUnsigned: { // uint32
                uint32_t val = (response[pos] << 24) | (response[pos + 1] << 16) | 
                            (response[pos + 2] << 8) | response[pos + 3];
                return val;
            }

            case DlmsTag::Null:
                return std::string("(None/Null)");

            default:
                throw std::runtime_error("Unsupported DLMS tag: " + std::to_string(static_cast<uint8_t>(tag)));
        }
    }

} // namespace opendlms