#include <iostream>
#include <iomanip>
#include "opendlms/dlms_client.hpp"
#include "opendlms/transport/tcp_client.hpp"

using namespace opendlms;

int main() {
    // 1. Setup Connection (Client 16 -> Server 1)
    TcpClient media("localhost", 4050, 2000);
    DlmsClient client(0x10, 0x01, TransportType::HDLC);

    if (!media.connect()) {
        std::cerr << "TCP Connection Failed" << std::endl;
        return 1;
    }
    std::cout << "TCP Connected." << std::endl;

    // --- STEP 1: SNRM Handshake (Link Layer Setup) ---
    // Resets sequence numbers to 0.
    auto snrm = client.snrmRequest();
    auto ua_bytes = media.sendAndReceive(snrm);
    
    if (!client.parseUaResponse(ua_bytes)) {
        std::cerr << "SNRM Failed! Check Simulator." << std::endl;
        return -1;
    }
    std::cout << "HDLC Link Established." << std::endl;

    // --- STEP 2: AARQ (Application Association) ---
    std::cout << "Sending AARQ..." << std::endl;
    
    // Generate AARQ frame (N(S)=0, N(R)=0)
    auto aarq_frame = client.aarqRequest(); 
    
    // Send and WAIT for AARE (Association Response)
    auto aare_bytes = media.sendAndReceive(aarq_frame);

    if (aare_bytes.empty()) {
        std::cerr << "Error: No AARE received. Simulator rejected the frame." << std::endl;
        return -1;
    }

    // Process AARE: This updates RxCount to 1 (syncing with Simulator)
    client.processResponse(aare_bytes);
    std::cout << "Association Established." << std::endl;

    // --- STEP 3: GET Request ---
    std::cout << "Reading Clock Object..." << std::endl;
    
    // Now generates Control 0x32 (N(S)=1, N(R)=1)
    // 0.0.1.0.0.255 is the OBIS code for Clock (Blue Book 4.2.7) [cite: 6609]
    auto get_req = client.getRequest(8, "0.0.1.0.0.255", 2); 
    
    auto get_resp = media.sendAndReceive(get_req);
    client.processResponse(get_resp);

    std::cout << "Data Received: ";
    for(auto b : get_resp) std::cout << std::hex << (int)b << " ";
    std::cout << std::endl;

    return 0;
}