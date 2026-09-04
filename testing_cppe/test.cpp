// g++ -std=c++23 test.cpp -o test -lstdc++exp -lws2_32
#include <iostream>
#include <string>
#include <experimental/net>

namespace net = std::experimental::net;

int main() {
    try {
        net::io_context io;

        net::ip::tcp::resolver resolver(io);
        net::ip::tcp::tcp::resolver::results_type endpoints = resolver.resolve("google.com", "80"); // host, service

        for (const auto& endpoint : endpoints) {
            std::cout << "Resolved: " << endpoint.endpoint() << "\n";
        }

        net::ip::tcp::socket socket(io);

        auto endpoint = endpoints.begin()->endpoint();
        
        socket.open(endpoint.protocol());
        socket.connect(endpoint);

        std::string request = "GET / HTTP/1.1\r\nHost: google.com\r\nConnection: close\r\n";
        socket.send(net::buffer(request));

        char buffer[4096];
        
        for (;;) {
            std::error_code ec;
            auto n = socket.receive(net::buffer(buffer), static_cast<net::socket_base::message_flags>(0), ec);

            if (n) {
                std::cout.write(buffer, n);
            }

            if (ec == net::stream_errc::eof) {
                break;
            }

            if (ec) {
                throw std::system_error(ec);
            }
        }
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}