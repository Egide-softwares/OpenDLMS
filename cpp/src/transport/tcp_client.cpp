#include "opendlms/transport/tcp_client.hpp"
#include <asio/steady_timer.hpp>
#include <iostream>
#include <thread> 
#include <chrono>

namespace opendlms {
    TcpClient::TcpClient(const std::string& host, uint16_t port, int timeout_ms)
    : host_(host), port_(port), timeout_ms_(timeout_ms), 
      socket_(io_context_), deadline_(io_context_) {}

    bool TcpClient::connect() {
        try {
            asio::ip::tcp::resolver resolver(io_context_);
            asio::connect(socket_, resolver.resolve(host_, std::to_string(port_)));
            
            // Disable Nagle's algorithm for DLMS real-time frames
            socket_.set_option(asio::ip::tcp::no_delay(true));
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Connection error: " << e.what() << std::endl;
            return false;
        }
    }

    void TcpClient::disconnect() {
        if (socket_.is_open()) socket_.close();
    }

    std::vector<uint8_t> TcpClient::sendAndReceive(const std::vector<uint8_t>& data) {
        std::vector<uint8_t> response;
        std::error_code read_ec = asio::error::would_block;
        std::size_t total_bytes = 0;
        std::vector<uint8_t> read_buffer(2048);

        // 1. Set the timer
        deadline_.expires_after(std::chrono::milliseconds(timeout_ms_));
        deadline_.async_wait([&](const std::error_code& ec) {
            if (!ec) {
                socket_.cancel(); // Timer finished first, kill the socket operation
            }
        });

        try {
            // 2. Write data (Synchronous is fine for small DLMS frames)
            asio::write(socket_, asio::buffer(data));

            // 3. Start Async Read
            socket_.async_read_some(asio::buffer(read_buffer),
                [&](const std::error_code& ec, std::size_t length) {
                    read_ec = ec;
                    total_bytes = length;
                });

            // 4. Run the context until a callback completes
            io_context_.restart();
            while (io_context_.run_one()) {
                if (read_ec != asio::error::would_block) {
                    deadline_.cancel(); // Data arrived, stop the clock!
                }
            }

            if (!read_ec && total_bytes > 0) {
                response.assign(read_buffer.begin(), read_buffer.begin() + total_bytes);
            } else if (read_ec == asio::error::operation_aborted) {
                std::cerr << "Communication Timeout after " << timeout_ms_ << "ms" << std::endl;
            }

        } catch (const std::exception& e) {
            std::cerr << "Transport Error: " << e.what() << std::endl;
        }

        return response;
    }

} // namespace opendlms