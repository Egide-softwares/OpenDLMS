#pragma once
#include <asio.hpp>
#include <vector>
#include <string>

namespace opendlms {

    class TcpClient {
        public:
            // TCP Client implementation for communication with the DLMS meter
            TcpClient(const std::string& host, uint16_t port, int timeout_ms = 3000);

            // Connects to the TCP server for DLMS meter communication
            bool connect();

            // Disconnects from the TCP IP server
            void disconnect();

            // Sends data and waits for a response (Synchronous)
            std::vector<uint8_t> sendAndReceive(const std::vector<uint8_t>& data);

        private:
            asio::io_context io_context_; // I/O context
            asio::ip::tcp::socket socket_; // TCP socket
            std::string host_; // Server host/IP (e.g., "localhost")
            uint16_t port_; // Server port (e.g., 4050)
            int timeout_ms_; // Timeout for send/receive operations in milliseconds
            asio::steady_timer deadline_; // Timer for handling timeouts
    };

} // namespace opendlms