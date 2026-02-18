#include "opendlms/dlms_client.hpp"
#include "opendlms/transport/tcp_client.hpp"
#include "opendlms/dlms/axdr_decoder.hpp"
#include "opendlms/common/utils.hpp"
#include <iostream>
#include <thread>
#include <chrono>

using namespace opendlms;

void read_meter(TcpClient& media, DlmsClient& client, const std::string& name, uint16_t classId, const std::string& obis, uint8_t attr) {
    std::cout << "Reading " << name << " (" << obis << ")..." << std::endl;
    auto req = client.getRequest(classId, obis, attr);
    auto res = media.sendAndReceive(req);
    client.processResponse(res); // Important for HDLC sequence sync

    if (!res.empty()) {
        try {
            DlmsValue val = AxdrDecoder::decode(res);
            if (std::holds_alternative<std::string>(val)) 
                std::cout << name << ": " << std::get<std::string>(val) << std::endl;
            else if (std::holds_alternative<uint32_t>(val))
                std::cout << name << ": " << std::get<uint32_t>(val) << std::endl;
            else if (std::holds_alternative<uint16_t>(val))
                std::cout << name << ": " << std::get<uint16_t>(val) << std::endl;
        } catch (const std::exception& e) { std::cout << name << ": (" << e.what() << ")" << std::endl; }
    } else {
        std::cout << name << ": [No Response]" << std::endl;
    }
}

int main() {
    TcpClient media("localhost", 4050, 5000);
    DlmsClient client(0x10, 0x01, TransportType::HDLC);

    std::cout << "\n--- STEP 1: TCP Connect ---" << std::endl;
    if (!media.connect()) return 1;
    else std::cout << "TCP Connection Established." << std::endl;

    std::cout << "\n--- STEP 2: HDLC SNRM Handshake ---" << std::endl;
    auto ua = media.sendAndReceive(client.snrmRequest());
    if (!client.parseUaResponse(ua)) {
        std::cerr << "UA Response Failed!" << std::endl;
        return 1;
    }
    std::cout << "HDLC Link Established." << std::endl;

    std::cout << "\n--- STEP 3: AARQ Association ---" << std::endl;
    auto aare = media.sendAndReceive(client.aarqRequest());
    client.processResponse(aare);
    std::cout << "Association Successful." << std::endl;

    std::cout << "\nReading Meter Data..." << std::endl;
    while (true) {
        std::cout << "\n--- Meter Read (HDLC Mode) ---" << std::endl;
        read_meter(media, client, "Device ID", 1, "0.0.96.1.0.255", 2);
        read_meter(media, client, "Logical Device Name", 1, "0.0.42.0.0.255", 2);
        read_meter(media, client, "Invocation Counter", 1, "0.0.43.1.2.255", 2);
        read_meter(media, client, "Clock", 8, "0.0.1.0.0.255", 2);
        read_meter(media, client, "Energy", 3, "1.0.1.8.0.255", 2);
        read_meter(media, client, "Voltage", 3, "1.0.32.7.0.255", 2);
        
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    media.disconnect();
    return 0;
}