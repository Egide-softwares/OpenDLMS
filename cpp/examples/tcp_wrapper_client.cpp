#include "opendlms/dlms_client.hpp"
#include "opendlms/transport/tcp_client.hpp"
#include "opendlms/dlms/axdr_decoder.hpp"
#include <iostream>
#include <thread>

using namespace opendlms;

// Helper to match the python-style reading
void read_meter_wrapper(TcpClient& media, DlmsClient& client, const std::string& name, uint16_t classId, const std::string& obis, std::string unit = "") {
    auto res = media.sendAndReceive(client.getRequest(classId, obis, 2));
    if (!res.empty()) {
        try {
            DlmsValue val = AxdrDecoder::decode(res);
            std::cout << name << ": ";
            if (std::holds_alternative<std::string>(val)) std::cout << std::get<std::string>(val);
            else if (std::holds_alternative<uint32_t>(val)) std::cout << std::get<uint32_t>(val);
            std::cout << " " << unit << std::endl;
        } catch (const std::exception& e) { std::cout << name << ": (" << e.what() << ")" << std::endl; }
    }
}

int main() {
    TcpClient media("localhost", 4051, 5000);
    DlmsClient client(0x10, 0x01, TransportType::WRAPPER);

    std::cout << "\n--- STEP 1: TCP Connect (Wrapper) ---" << std::endl;
    if (!media.connect()) return 1;

    std::cout << "\n--- STEP 2: AARQ Association ---" << std::endl;
    auto aare = media.sendAndReceive(client.aarqRequest());
    if (aare.empty()) {
        std::cerr << "Wrapper Association Failed. Is simulator running with -w?" << std::endl;
        return 1;
    }
    std::cout << "Wrapper Association Successful." << std::endl;

    std::cout << "\nReading Meter Data..." << std::endl;
    while (true) {
        std::cout << "\n--- Meter Read (Wrapper Mode) ---" << std::endl;
        read_meter_wrapper(media, client, "Device ID", 1, "0.0.96.1.0.255");
        read_meter_wrapper(media, client, "Logical Device Name", 1, "0.0.42.0.0.255");
        read_meter_wrapper(media, client, "Invocation Counter", 1, "0.0.43.1.2.255");
        read_meter_wrapper(media, client, "Clock", 8, "0.0.1.0.0.255");
        read_meter_wrapper(media, client, "Energy", 3, "1.0.1.8.0.255", "kWh");
        read_meter_wrapper(media, client, "Voltage", 3, "1.0.32.7.0.255", "V");

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    return 0;
}