#include <algorithm>
#include "opendlms/dlms_client.hpp"
#include "opendlms/common/crc.hpp"
#include "opendlms/common/obis.hpp"

namespace opendlms {

    DlmsClient::DlmsClient(uint16_t clientAddr, uint16_t serverAddr, TransportType type)
        : clientAddr_(clientAddr), serverAddr_(serverAddr), transportType_(type) {}

    uint8_t DlmsClient::getNextControlField() {
        // [ N(R) | P | N(S) | 0 ] -> 0x10 is Poll bit
        uint8_t control = (rx_count_ << 5) | 0x10 | (tx_count_ << 1);
        tx_count_ = (tx_count_ + 1) % 8;
        return control;
    }

    void DlmsClient::resetSequence() {
        // N(S): Our Send Sequence number
        tx_count_ = 0;
        
        // N(R): Our Receive Sequence number (what we expect from the server)
        rx_count_ = 0;
        
        // In Wrapper mode, these variables exist but are ignored by wrapInWrapper()
    }

    std::vector<uint8_t> DlmsClient::snrmRequest() {
        if (transportType_ == TransportType::WRAPPER) return {};
        
        std::vector<uint8_t> frame = {0xA0, 0x07, 
                                    static_cast<uint8_t>(serverAddr_), 
                                    static_cast<uint8_t>(clientAddr_), 
                                    0x93}; // SNRM
        uint16_t fcs = utils::Crc16::calculate(frame.data(), frame.size());
        
        std::vector<uint8_t> full = {0x7E};
        full.insert(full.end(), frame.begin(), frame.end());
        full.push_back(fcs & 0xFF);
        full.push_back(fcs >> 8);
        full.push_back(0x7E);
        return full;
    }

    bool DlmsClient::parseUaResponse(const std::vector<uint8_t>& response) {
        // A valid UA frame for Client 16/Server 1 is at least 7 bytes: 
        // 7E (flag) + A0 (format) + Len + Dest + Src + 73 (UA) + FCS + 7E (flag)
        // However, most UA frames include parameters, making them ~20-30 bytes.
        if (response.size() < 7) return false;

        // 1. Check for HDLC Frame Format (A0 or A8)
        if ((response[1] & 0xF0) != 0xA0) return false;

        // 2. Verify Control Field is UA (0x73 or 0x63 depending on Poll bit)
        // In your logs, the simulator returned 0x73.
        uint8_t control = 0;
        
        // The control field index changes if addresses are more than 1 byte.
        // For our current 1-byte addressing (03 and 21), it is at index 5.
        control = response[5];

        if (control != 0x73 && control != 0x63) {
            return false;
        }

        // 3. Optional: Parse negotiated parameters (Max Info Field Length)
        // Your UA had: ... 81 80 12 05 01 80 ...
        // 0x80 is the 'User Information' tag. 
        // This is where you would extract the meter's buffer limits.
        
        return true; 
    }

    void DlmsClient::processResponse(const std::vector<uint8_t>& response) {
        if (transportType_ == TransportType::HDLC && response.size() > 5) {
            uint8_t control = response[5];
            if ((control & 0x01) == 0) {
                rx_count_ = ((control >> 1) & 0x07) + 1;
                rx_count_ %= 8;
            }
        }
    }

    std::vector<uint8_t> DlmsClient::wrapInWrapper(const std::vector<uint8_t>& apdu) {
        std::vector<uint8_t> frame = {0x00, 0x01}; // Protocol ID
        frame.push_back(clientAddr_ >> 8); frame.push_back(clientAddr_ & 0xFF);
        frame.push_back(serverAddr_ >> 8); frame.push_back(serverAddr_ & 0xFF);
        frame.push_back(apdu.size() >> 8); frame.push_back(apdu.size() & 0xFF);
        frame.insert(frame.end(), apdu.begin(), apdu.end());
        return frame;
    }

    std::vector<uint8_t> DlmsClient::wrapInHdlc(const std::vector<uint8_t>& apdu, uint8_t control) {
        uint16_t frameLen = apdu.size() + 12; 
        std::vector<uint8_t> header = {
            static_cast<uint8_t>(0xA0 | ((frameLen >> 8) & 0x07)),
            static_cast<uint8_t>(frameLen & 0xFF),
            static_cast<uint8_t>(serverAddr_),
            static_cast<uint8_t>(clientAddr_),
            control
        };

        uint16_t hcs = utils::Crc16::calculate(header.data(), header.size());
        std::vector<uint8_t> body = header;
        body.push_back(hcs & 0xFF); body.push_back(hcs >> 8);
        body.push_back(0xE6); body.push_back(0xE6); body.push_back(0x00);
        body.insert(body.end(), apdu.begin(), apdu.end());

        uint16_t fcs = utils::Crc16::calculate(body.data(), body.size());
        std::vector<uint8_t> frame = {0x7E};
        frame.insert(frame.end(), body.begin(), body.end());
        frame.push_back(fcs & 0xFF); frame.push_back(fcs >> 8);
        frame.push_back(0x7E);
        return frame;
    }

    std::vector<uint8_t> DlmsClient::aarqRequest() {
        std::vector<uint8_t> apdu = {
            0x60, 0x1D, 0xA1, 0x09, 0x06, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x01, 0x01,
            0xBE, 0x10, 0x04, 0x0E, 0x01, 0x00, 0x00, 0x00, 0x06, 
            0x5F, 0x1F, 0x04, 0x00, 0x40, 0x1E, 0x5D, 0xFF, 0xFF
        };

        if (transportType_ == TransportType::WRAPPER) return wrapInWrapper(apdu);
        resetSequence();
        return wrapInHdlc(apdu, getNextControlField());
    }

    std::vector<uint8_t> DlmsClient::getRequest(uint16_t classId, const std::string& obis, uint8_t attribute) {
        std::vector<uint8_t> apdu = {0xC0, 0x01, 0xC1};
        apdu.push_back(classId >> 8);
        apdu.push_back(classId & 0xFF);

        // Use the ObisCode constructor directly from your obis.hpp
        ObisCode obisObj(obis);
        apdu.insert(apdu.end(), obisObj.bytes.begin(), obisObj.bytes.end());

        apdu.push_back(attribute);
        apdu.push_back(0x00);

        if (transportType_ == TransportType::WRAPPER) return wrapInWrapper(apdu);
        return wrapInHdlc(apdu, getNextControlField());
    }

}
