#include "opendlms/dlms/apdu_factory.hpp"
#include "opendlms/common/obis.hpp"
#include <sstream>
#include <vector>

namespace opendlms {

    std::vector<uint8_t> ApduFactory::createGetRequest(
        uint8_t invokeId, 
        uint16_t classId, 
        const std::string& obisStr, 
        uint8_t attributeId) 
    {
        std::vector<uint8_t> pdu;
        pdu.push_back(0xC0); // Tag: Get-Request
        pdu.push_back(0x01); // Choice: Get-Request-Normal
        pdu.push_back(invokeId);

        // 1. Interface Class ID (2 bytes)
        pdu.push_back(static_cast<uint8_t>(classId >> 8));
        pdu.push_back(static_cast<uint8_t>(classId & 0xFF));

        // 2. OBIS Code (6 bytes)
        ObisCode obisObj(obisStr);
        pdu.insert(pdu.end(), obisObj.bytes.begin(), obisObj.bytes.end());

        // 3. Attribute ID (1 byte)
        pdu.push_back(attributeId);

        // 4. Data Access Selection (Optional: 00 for none)
        pdu.push_back(0x00);

        return pdu;
    }

} // namespace opendlms