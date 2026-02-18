#include "opendlms/dlms/aare_parser.hpp"

namespace opendlms {

    AssociationResult AareParser::parse(const std::vector<uint8_t>& pdu) {
        if (pdu.empty() || pdu[0] != 0x61) {
            return AssociationResult::Error;
        }

        // A common "minimal" success response is 61 03 00 00 00
        if (pdu.size() == 5 && pdu[2] == 0x00) {
            return AssociationResult::Accepted;
        }

        // Search for the standard Result tag 0xA2
        for (size_t i = 0; i < pdu.size() - 4; ++i) {
            if (pdu[i] == 0xA2 && pdu[i+1] == 0x03) {
                uint8_t resultValue = pdu[i+4];
                return (resultValue == 0x00) ? AssociationResult::Accepted 
                                            : AssociationResult::Rejected;
            }
        }

        // If we got an AARE (0x61) but no explicit rejection tag was found, 
        // it's often a successful minimal association.
        return AssociationResult::Accepted;
    }

} // namespace opendlms