#pragma once
#include <vector>
#include <cstdint>
#include <string>

namespace opendlms {

    enum class TransportType {
        HDLC,
        WRAPPER
    };

    class DlmsClient {
        public:
            // Constructor takes client and server addresses, and optionally the transport type (default is WRAPPER)
            DlmsClient(uint16_t clientAddr, uint16_t serverAddr, TransportType type = TransportType::WRAPPER);

            // ------Configuration-------
            
            // Set the transport type (HDLC or Wrapper)
            void setTransportType(TransportType type) { transportType_ = type; }

            // ------HDLC Layer----------

            // Returns the SNRM frame to initiate HDLC association
            std::vector<uint8_t> snrmRequest();
            // Returns true if the frame is a valid UA response to our SNRM
            bool parseUaResponse(const std::vector<uint8_t>& response);

            // ------Application Layer----

            // Constructs an AARQ request APDU for association (Wrapper or HDLC based on transport type)
            std::vector<uint8_t> aarqRequest();
            // Constructs a GET request APDU for the specified class, OBIS code, and attribute
            std::vector<uint8_t> getRequest(uint16_t classId, const std::string& obis, uint8_t attribute);
            // Parses a response APDU and updates internal state (e.g., sequence numbers for HDLC)
            void processResponse(const std::vector<uint8_t>& response);

            /**
             * @brief Resets HDLC sequence numbers N(S) and N(R) to zero.
             * This must be called after a UA response or before initiating 
             * a new Application Association (AARQ) in HDLC mode.
             */
            void resetSequence();

        private:
            uint16_t clientAddr_; // Client address (e.g., 0x10 for 16)
            uint16_t serverAddr_; // Server address (e.g., 0x01 for 1)
            TransportType transportType_; // Transport type (HDLC or Wrapper)

            // HDLC State/Sequence Tracking
            uint8_t tx_count_ = 0; // N(S)
            uint8_t rx_count_ = 0; // N(R)

            // Internal Framing Helpers
            uint8_t getNextControlField();
            std::vector<uint8_t> wrapInHdlc(const std::vector<uint8_t>& apdu, uint8_t control);
            std::vector<uint8_t> wrapInWrapper(const std::vector<uint8_t>& apdu);
    };

}
