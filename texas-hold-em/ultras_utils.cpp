#include <cstdint>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <conio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "ultras_utils.hpp"

namespace uul {
    namespace ANSI {
        std::string moveUp(int n = 1) {
            return "\x1b[" + std::to_string(n) + "A";
        }
    }

    #pragma region Server

    Server::Server() {
        Server::Initialize();
    }

    bool Server::Initialize() {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            return false;
        }

        client_socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (client_socket_ == INVALID_SOCKET) {
            WSACleanup();
            return false;
        }

        char broadcast_on = 1;
        if (setsockopt(client_socket_, SOL_SOCKET, SO_BROADCAST, &broadcast_on, sizeof(broadcast_on)) == SOCKET_ERROR) {
            closesocket(client_socket_);
            WSACleanup();
            return false;
        }

        return true;
    }

    void Server::Cleanup() {
        if (client_socket_ != INVALID_SOCKET) {
            closesocket(client_socket_);
            client_socket_ = INVALID_SOCKET;
        }

        if (listen_socket_ != INVALID_SOCKET) {
            closesocket(listen_socket_);
            listen_socket_ = INVALID_SOCKET;
        }

        if (udp_discovery_socket_ != INVALID_SOCKET) {
            closesocket(udp_discovery_socket_);
            udp_discovery_socket_ = INVALID_SOCKET;
        }

        WSACleanup();

        std::cout << "Winsock cleanup complete." << std::endl;
    }

    bool Server::CreateServer(int port) {
        if (listen_socket_ != INVALID_SOCKET) {
            std::cerr << "Server is already listening." << std::endl;
            return false;
        }

        listen_socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listen_socket_ == INVALID_SOCKET) {
            std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
            return false;
        }

        sockaddr_in service;
        service.sin_family = AF_INET;
        service.sin_addr.s_addr = htonl(INADDR_ANY);
        service.sin_port = htons(port);

        if (bind(listen_socket_, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
            std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
            closesocket(listen_socket_);
            listen_socket_ = INVALID_SOCKET;
            return false;
        }

        if (listen(listen_socket_, SOMAXCONN) == SOCKET_ERROR) {
            std::cerr << "Listen failed with error: " << WSAGetLastError() << std::endl;
            closesocket(listen_socket_);
            listen_socket_ = INVALID_SOCKET;
            return false;
        }

        std::cout << "Server successfully started and listening on port " << port << "." << std::endl;
        return true;
    }

    SOCKET Server::AcceptClient() {
        SOCKET accepted_client_socket = INVALID_SOCKET;
        sockaddr_in client_info;
        int client_info_size = sizeof(client_info);

        if (listen_socket_ == INVALID_SOCKET) {
            std::cerr << "Error: Cannot accept client. Listening socket is not active." << std::endl;
            return INVALID_SOCKET;
        }

        std::cout << "Waiting for a new client connection..." << std::endl;
        
        accepted_client_socket = accept(listen_socket_, (SOCKADDR*)&client_info, &client_info_size);

        if (accepted_client_socket == INVALID_SOCKET) {
            std::cerr << "Accept failed with error: " << WSAGetLastError() << std::endl;
            return INVALID_SOCKET;
        }

        char client_ip[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &(client_info.sin_addr), client_ip, INET_ADDRSTRLEN)) {
            std::cout << "Client accepted from IP: " << client_ip << std::endl;
        }

        return accepted_client_socket;
    }

    bool Server::ConnectClient(const std::string& ip, int port) {
        if (client_socket_ != INVALID_SOCKET) {
            closesocket(client_socket_);
            client_socket_ = INVALID_SOCKET;
        }

        client_socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (client_socket_ == INVALID_SOCKET) {
            std::cerr << "Error creating client socket: " << WSAGetLastError() << std::endl;
            return false;
        }

        sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        
        if (inet_pton(AF_INET, ip.c_str(), &(server_addr.sin_addr)) != 1) {
            std::cerr << "Error converting IP address: " << ip << std::endl;
            closesocket(client_socket_);
            client_socket_ = INVALID_SOCKET;
            return false;
        }

        std::cout << "Attempting to connect to " << ip << ":" << port << "..." << std::endl;

        if (connect(client_socket_, (SOCKADDR*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
            std::cerr << "Connection failed with error: " << WSAGetLastError() << std::endl;
            closesocket(client_socket_);
            client_socket_ = INVALID_SOCKET;
            return false;
        }

        std::cout << "Successfully connected to game server!" << std::endl;
        return true;
    }

    std::vector<std::string> Server::SearchForLANGames() {
        std::vector<std::string> active_ips;

        sockaddr_in broadcast_addr;
        broadcast_addr.sin_family = AF_INET;
        broadcast_addr.sin_port = htons(DEFAULT_PORT);
        broadcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

        const char* query_message = "LAN_GAME_SEARCH";
        if (sendto(client_socket_, query_message, (int)strlen(query_message), 0, (SOCKADDR*)&broadcast_addr, sizeof(broadcast_addr)) == SOCKET_ERROR) {
            return active_ips;
        }

        DWORD timeout_ms = 5000;
        setsockopt(client_socket_, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout_ms, sizeof(timeout_ms));

        char recv_buf[512];
        sockaddr_in server_addr;
        int server_addr_size = sizeof(server_addr);

        while (true) {
            int bytes_received = recvfrom(client_socket_, recv_buf, sizeof(recv_buf), 0, (SOCKADDR*)&server_addr, &server_addr_size);

            if (bytes_received > 0) {
                char ip_str[INET_ADDRSTRLEN];
                if (inet_ntop(AF_INET, &(server_addr.sin_addr), ip_str, INET_ADDRSTRLEN)) {
                    active_ips.push_back(std::string(ip_str));
                }
            } else if (bytes_received == 0) {
                break;
            } else {
                int error = WSAGetLastError();
                if (error == WSAETIMEDOUT) {
                    break;
                } else {
                    break;
                }
            }
        }

        return active_ips;
    }

    bool Server::StartDiscoveryListener(int port) {
        std::vector<int> test;

        if (udp_discovery_socket_ != INVALID_SOCKET) {
            std::cerr << "UDP listener already running or socket not cleaned up." << std::endl;
            return false;
        }

        udp_discovery_socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (udp_discovery_socket_ == INVALID_SOCKET) {
            std::cerr << "Error creating UDP discovery socket: " << WSAGetLastError() << std::endl;
            return false;
        }

        sockaddr_in host_addr;
        host_addr.sin_family = AF_INET;
        host_addr.sin_port = htons(port);
        host_addr.sin_addr.s_addr = htonl(INADDR_ANY);

        if (bind(udp_discovery_socket_, (SOCKADDR*)&host_addr, sizeof(host_addr)) == SOCKET_ERROR) {
            std::cerr << "UDP Bind failed with error: " << WSAGetLastError() << std::endl;
            closesocket(udp_discovery_socket_);
            udp_discovery_socket_ = INVALID_SOCKET;
            return false;
        }

        std::cout << "UDP discovery listener started on port " << port << "." << std::endl;
        return true;
    }

    void Server::RunDiscoveryLoop() {
        if (udp_discovery_socket_ == INVALID_SOCKET) {
            std::cerr << "Cannot run discovery loop: Socket is invalid." << std::endl;
            return;
        }
        
        char recv_buf[512];
        sockaddr_in client_addr;
        int client_addr_size = sizeof(client_addr);
        const std::string SERVER_INFO = "My Ultra Game Server"; 
        
        std::cout << "UDP discovery loop is active." << std::endl;

        while (true) { 
            int bytes_received = recvfrom(udp_discovery_socket_, recv_buf, sizeof(recv_buf), 0, (SOCKADDR*)&client_addr, &client_addr_size);

            if (bytes_received > 0) {
                recv_buf[bytes_received] = '\0';
                
                if (strcmp(recv_buf, "LAN_GAME_SEARCH") == 0) {
                    const char* reply_data = SERVER_INFO.c_str();
                    sendto(udp_discovery_socket_, reply_data, (int)SERVER_INFO.length(), 0, (SOCKADDR*)&client_addr, client_addr_size);
                }
            } else if (bytes_received == SOCKET_ERROR) {
                int error = WSAGetLastError();
                if (error == WSAEINTR || error == WSAENOTSOCK) { 
                    break;
                } else {
                    std::cerr << "Discovery listener error: " << error << std::endl;
                    break; 
                }
            }
        }
    }

    bool Server::SendRequest(SOCKET s, const std::string& message) {
        if (s == INVALID_SOCKET) return false;

        uint32_t length = static_cast<uint32_t>(message.length());
        uint32_t net_length = htonl(length);

        int bytes_sent = send(s, reinterpret_cast<const char*>(&net_length), sizeof(net_length), 0);
        if (bytes_sent == SOCKET_ERROR) {
            std::cerr << "Failed to send message length: " << WSAGetLastError() << std::endl;
            return false;
        }

        if (length > 0) {
            bytes_sent = send(s, message.c_str(), length, 0);
            if (bytes_sent == SOCKET_ERROR) {
                std::cerr << "Failed to send message body: " << WSAGetLastError() << std::endl;
                return false;
            }
        }

        return true;
    }

    std::string Server::ReceiveRequest(SOCKET s) {
        if (s == INVALID_SOCKET) return "";

        uint32_t net_length = 0;
        int bytes_received = recv(s, reinterpret_cast<char*>(&net_length), sizeof(net_length), 0);
        
        if (bytes_received <= 0) {
            return "";
        }

        uint32_t length = ntohl(net_length);
        if (length == 0) return "";

        std::string message;
        message.resize(length);

        uint32_t total_received = 0;
        while (total_received < length) {
            bytes_received = recv(s, &message[total_received], length - total_received, 0);
            if (bytes_received <= 0) {
                std::cerr << "Error receiving message body or connection closed." << std::endl;
                return "";
            }
            total_received += bytes_received;
        }

        return message;
    }

    #pragma endregion Server

    #pragma region CLI

    CLI::CLI() {
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    }

    void CLI::goToXY(int x, int y) {
        COORD coord;
        coord.X = x;
        coord.Y = y;
        SetConsoleCursorPosition(hConsole, coord);
    }

    COORD CLI::getCursorPosition() {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
            return csbi.dwCursorPosition;
        }
        return {0,0};
    }

    void CLI::EnableUTF8() {
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
    }

    std::string CLI::MultipleChoice(const std::vector<std::string>& options, const std::string& question) {
        if (options.empty()) return "";

        int selected = 0;
        COORD startPos = getCursorPosition();

        auto render = [&]() {
            goToXY(0, startPos.Y);
            std::cout << ANSI::CLEAR_LINE << question << std::endl;

            for (size_t i = 0; i < options.size(); i++) {
                std::cout << ANSI::CLEAR_LINE;
                const std::string prefix = (selected == i) ? ANSI::CYAN + "> " + ANSI::RESET : "  ";
                std::cout << prefix << options[i] << std::endl;
            }
            std::cout.flush();
        };

        render();

        while (true) {
            int key = _getch();

            if (key == 0 || key == 0xE0) {
                key = _getch();
                if (key == 72) {
                    selected = (selected - 1 + options.size()) % options.size();
                    render();
                } else if (key == 80) {
                    selected = (selected + 1) % options.size();
                    render();
                }
            } else if (key == ' ' || key == '\n' || key == '\r') {
                break;
            } else if (key == 3) {
                exit(0);
            }
        }

        goToXY(0, startPos.Y);

        size_t totalLines = options.size() + 1;
        for(size_t i = 0; i < totalLines; ++i) {
            goToXY(0, startPos.Y + i);
            std::cout << ANSI::CLEAR_LINE;
        }
        
        goToXY(0, startPos.Y); 

        return options[selected];
    }

    #pragma endregion CLI

    #pragma region File

    std::string File::ReadFile(const std::string& path) {
        std::string fileData;
        std::ifstream is(path);

        if (!is.is_open()) {
            std::cerr << "Error: Could not open file for static reading: " << path << std::endl;
            return "";
        }

        try {
            std::stringstream buffer;
            buffer << is.rdbuf();
            fileData = buffer.str();
        } catch (const std::exception& e) {
            std::cerr << "Error during static file reading: " << e.what() << std::endl;
            return "";
        }

        return fileData;
    }

    bool File::WriteFile(std::string& path, const std::string& data) {
        std::ofstream os(path, std::ios_base::trunc);
        
        if (!os.is_open()) {
            std::cerr << "Error: Could not open file for static writing: " << path << std::endl;
            return false;
        }

        if (os << data) {
            return true;
        } else {
            std::cerr << "Error: Failed to perform statuc file write: " << path << std::endl;
            return false;
        }
    }

    #pragma endregion File
}