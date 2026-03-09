#include <iostream>
#include <iomanip>
#include <algorithm>
#include "opendlms/dlms_client.hpp"
#include "opendlms/common/crc.hpp"
#include "opendlms/common/obis.hpp"
#include "opendlms/common/utils.hpp"
#include "opendlms/dlms/aarq_builder.hpp"

namespace opendlms {

    DlmsClient::DlmsClient(uint16_t clientAddr, uint16_t serverAddr, TransportType type)
        : clientAddr_(clientAddr), serverAddr_(serverAddr), transportType_(type), 
          tx_count_(0), rx_count_(0) {}

    uint8_t DlmsClient::getRxCount() const { return rx_count_; }

    /**
     * Generates HDLC Control Field. 
     * Formula: (N(R) << 5) | (Poll=1 << 4) | (N(S) << 1) | (FrameType=0)
     */
    uint8_t DlmsClient::getNextControlField(bool isCommand) {
        uint8_t control = (rx_count_ << 5) | (1 << 4) | (tx_count_ << 1);
        
        // Gurux Logic: Increment TX only when sending a command (I-Frame)
        if (isCommand) {
            tx_count_ = (tx_count_ + 1) % 8;
        }
        return control;
    }

    /**
     * CRITICAL FIX: Syncs N(R) with Simulator's N(S)
     * This logic is found in GuruxDLMS.c 'gx_updateSequence'
     */
    void DlmsClient::updateSequenceNumbers(uint8_t controlByte) {
        // If it's an I-Frame (Bit 0 is 0), it contains a sequence number
        if ((controlByte & 0x01) == 0) {
            uint8_t simulator_ns = (controlByte >> 1) & 0x07;
            
            // We acknowledge receiving Frame N by setting our N(R) to N+1
            rx_count_ = (simulator_ns + 1) % 8;
        }
    }

    std::vector<uint8_t> DlmsClient::snrmRequest() {
        if (transportType_ == TransportType::WRAPPER) return {};

        // HDLC Addressing: (Address << 1) | 1
        uint8_t hdlc_server = (serverAddr_ << 1) | 0x01;
        uint8_t hdlc_client = (clientAddr_ << 1) | 0x01;

        // Bare SNRM (Format 3, Length 7) - Matches Gurux Default
        std::vector<uint8_t> frame = {
            0xA0, 0x07, hdlc_server, hdlc_client, 0x93
        };

        // Header Check Sequence (HCS)
        uint16_t fcs = utils::Crc16::calculate(frame.data(), frame.size());
        
        std::vector<uint8_t> full = {0x7E};
        full.insert(full.end(), frame.begin(), frame.end());
        full.push_back(fcs & 0xFF);
        full.push_back(fcs >> 8);
        full.push_back(0x7E);

        return full;
    }

    bool DlmsClient::parseUaResponse(const std::vector<uint8_t>& response) {
        if (response.size() < 7) return false;
        
        // Control field for 1-byte addressing is at index 5
        uint8_t control = response[5];
        
        // UA (0x73) or UA with Final bit (0x63)
        if (control != 0x73 && control != 0x63) return false;

        // Reset Counters on Connection Start
        tx_count_ = 0;
        rx_count_ = 0;
        return true; 
    }

    void DlmsClient::processResponse(const std::vector<uint8_t>& response) {
        // Minimum HDLC I-Frame size validation
        if (response.size() < 9 || response.front() != 0x7E || response.back() != 0x7E) return;

        // FCS Verification
        size_t bodySize = response.size() - 4; 
        const uint8_t* bodyPtr = &response[1];
        uint16_t receivedFcs = response[response.size() - 3] | (response[response.size() - 2] << 8);
        
        if (utils::Crc16::calculate(bodyPtr, bodySize) == receivedFcs) {
            // SUCCESS: Update our N(R) based on what the simulator sent
            updateSequenceNumbers(response[5]);
        } else {
            std::cerr << "HDLC FCS Error! Frame discarded." << std::endl;
        }
    }

    std::vector<uint8_t> DlmsClient::wrapInHdlc(const std::vector<uint8_t>& apdu, uint8_t control) {
        uint8_t hdlc_server = (serverAddr_ << 1) | 0x01;
        uint8_t hdlc_client = (clientAddr_ << 1) | 0x01;

        uint16_t frameLen = static_cast<uint16_t>(apdu.size() + 12);
        
        std::vector<uint8_t> header = {
            static_cast<uint8_t>(0xA0 | ((frameLen >> 8) & 0x07)),
            static_cast<uint8_t>(frameLen & 0xFF),
            hdlc_server, hdlc_client, control
        };

        // HCS (Header Check)
        uint16_t hcs = utils::Crc16::calculate(header.data(), header.size());
        
        std::vector<uint8_t> body = header;
        body.push_back(hcs & 0xFF); body.push_back(hcs >> 8);
        
        // LLC (Required for AARQ/GET)
        body.push_back(0xE6); body.push_back(0xE6); body.push_back(0x00);
        body.insert(body.end(), apdu.begin(), apdu.end());

        // FCS (Frame Check)
        uint16_t fcs = utils::Crc16::calculate(body.data(), body.size());
        
        std::vector<uint8_t> frame = {0x7E};
        frame.insert(frame.end(), body.begin(), body.end());
        frame.push_back(fcs & 0xFF); frame.push_back(fcs >> 8); frame.push_back(0x7E);

        return frame;
    }

    std::vector<uint8_t> DlmsClient::aarqRequest() {
        if (transportType_ == TransportType::WRAPPER) return wrapInWrapper(AarqBuilder::buildWrapper());
        
        // AARQ is an I-Frame. We pass 'true' to increment tx_count AFTER generation.
        return wrapInHdlc(AarqBuilder::buildHdlc(), getNextControlField(true)); 
    }

    std::vector<uint8_t> DlmsClient::getRequest(uint16_t classId, const std::string& obis, uint8_t attribute) {
        std::vector<uint8_t> apdu = {0xC0, 0x01, 0xC1, (uint8_t)(classId >> 8), (uint8_t)(classId & 0xFF)};
        ObisCode obisObj(obis);
        apdu.insert(apdu.end(), obisObj.bytes.begin(), obisObj.bytes.end());
        apdu.push_back(attribute); apdu.push_back(0x00);

        if (transportType_ == TransportType::WRAPPER) return wrapInWrapper(apdu);
        
        // This will now use 0x32 (NR=1, NS=1) IF processResponse was called on AARE
        return wrapInHdlc(apdu, getNextControlField(true));
    }
    
    // ... wrapInWrapper implementation ...
    std::vector<uint8_t> DlmsClient::wrapInWrapper(const std::vector<uint8_t>& apdu) {
        // Implementation remains the same as your previous correct version
        std::vector<uint8_t> frame = {0x00, 0x01}; 
        frame.push_back(clientAddr_ >> 8); frame.push_back(clientAddr_ & 0xFF);
        frame.push_back(serverAddr_ >> 8); frame.push_back(serverAddr_ & 0xFF);
        frame.push_back(apdu.size() >> 8); frame.push_back(apdu.size() & 0xFF);
        frame.insert(frame.end(), apdu.begin(), apdu.end());
        return frame;
    }
}