#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <type_traits>
#include <iostream>
#include <sstream>
#include <fstream>
#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")

#ifdef ULTRAS_UTILS_STATIC
    #define ULTRAS_UTILS_API
#elif defined(ULTRAS_UTILS_EXPORTS)
    #define ULTRAS_UTILS_API __declspec(dllexport)
#else
    #define ULTRAS_UTILS_API __declspec(dllimport)
#endif

namespace uul {
    /**
     * @brief Core utility class for managing network operations
     * 
     * Contains all Winsock setup, server listening, and client connection
     * logic for a server
     */
    class ULTRAS_UTILS_API Server {
    public:
        Server();

        static constexpr int DEFAULT_PORT = 54000;
        static constexpr int DEFAULT_BUFFER_LENGTH = 512;

        /**
         * @brief Initializes the Winsock library (called first).
         * @return true on success, false on failure.
         */
        bool Initialize();

        /**
         * @brief Cleans up Winsock library (called last).
         */
        void Cleanup();

        /**
         * @brief Creates a simple TCP server and starts listening.
         * @param port The port to listen on.
         * @return true if the server is successfully listening, false otherwise.
         */
        bool CreateServer(int port = DEFAULT_PORT);

        /**
         * @brief Waits for and accepts a client connection (blocking).
         * @return The client socket handle on success, or INVALID_SOCKET on failure.
         */
        SOCKET AcceptClient();

        /**
         * @brief Connects to a server.
         * @param ip The server's IP address (e.g., "127.0.0.1" for localhost).
         * @param port The server's port.
         * @return true if successful connection, false otherwise.
         */
        bool ConnectClient(const std::string& ip, int port = DEFAULT_PORT);

        /**
         * @brief Sends a string message over a specific socket.
         */
        bool SendRequest(SOCKET s, const std::string& message);

        /**
         * @brief Receives a string message from a specific socket (blocking).
         */
        std::string ReceiveRequest(SOCKET s);
        SOCKET GetClientSocket() const { return client_socket_; }
        SOCKET GetListenSocket() const { return listen_socket_; }

        /**
         * @brief Searches for LAN games.
         * @return A vector of ip's running games.
         */
        std::vector<std::string> SearchForLANGames();

        /**
         * @brief Starts the UDP listener for discovery
         * @param port Port to listen on
         * @return True if successful, false otherwise.
         */
        bool StartDiscoveryListener(int port = DEFAULT_PORT);

        /**
         * @brief Run the loop to run discovery
         */
        void RunDiscoveryLoop();

    private:
        WSADATA wsa_data_;
        SOCKET listen_socket_ = INVALID_SOCKET;
        SOCKET client_socket_ = INVALID_SOCKET;
        SOCKET udp_discovery_socket_ = INVALID_SOCKET;
    };

    namespace ANSI {
        const std::string RESET = "\x1b[0m";
        const std::string GREY = "\x1b[90m";
        const std::string CYAN = "\x1b[36m";
        const std::string GREEN = "\x1b[32m";
        const std::string RED = "\x1b[31m";
        const std::string YELLOW = "\x1b[33m";
        const std::string MAGENTA = "\x1b[35m";
        const std::string CLEAR_LINE = "\x1b[2K";
        
        std::string ULTRAS_UTILS_API moveUp(int n);
    }

    class ULTRAS_UTILS_API CLI {
    public:
        CLI();

        /**
         * @brief Creates a multiple choice question with arrow key navigation.
         * @param options A vector of options.
         * @param question The question being asked.
         * @return The option selected.
         */
        std::string MultipleChoice(const std::vector<std::string>& options, const std::string& question);

        /**
         * @brief Enables UTF-8 characters in terminal.
         */
        static void EnableUTF8();

        /**
         * @brief Move caret to (X, Y)
         * @param x X value
         * @param y Y value
         */
        void goToXY(int x, int y);

        /**
         * @brief Get the current position of the caret
         * @return The position of the caret as a COORD (.X, .Y)
         */
        COORD getCursorPosition();

    private:
        HANDLE hConsole;
    };

    class ULTRAS_UTILS_API File {
    private:
        std::string path_;
        bool success_ = false;

    public:
        File(const std::string& path) : path_(path) {};

        /**
         * @brief Read a file from a path.
         * @param path The path of the file.
         * @return The file data.
         */
        static std::string ReadFile(const std::string& path);

        /**
         * @brief Write to a file.
         * @param path The path of the file.
         * @param data The data to write.
         * @return true if successful, false otherwise.
         */
        static bool WriteFile(std::string& path, const std::string& data);


        /**
         * @brief Whether the last operation was successful.
         * @return true if successful, false otherwise.
         */
        bool wasSuccessful() const { return success_; }

        /**
         * @brief Get the `ofstream` of the file.
         * @return File's `ofstream`.
         */
        std::ofstream getOutputStream() const { return std::ofstream(path_, std::ios_base::out); };

        /**
         * @brief Get the `ifstream` of the file.
         * @return File's `ifstream`.
         */
        std::ifstream getInputStream() const { return std::ifstream(path_); };

        operator bool() const { return !path_.empty(); };

        template <typename T>
        friend File& operator<<(File&, const T& data);

        template <typename T>
        friend File& operator>>(File& file, T& data);
    };

    template <typename T>
    File& operator<<(File& file, const T& data) {
        std::ofstream os(file.path_, std::ios_base::app);
        file.success_ = false;
        
        if (!os.is_open()) {
            std::cerr << "Error: Could not open file for writing (append): " << file.path_ << std::endl;
            return file;
        }

        if (os << data) {
            file.success_ = true;
        } else {
            std::cerr << "Error: Failed to write data to file: " << file.path_ << std::endl;
        }
        
        return file;
    }

    template <typename T>
    File& operator>>(File& file, T& data) {
        file.success_ = false;

        if constexpr (std::is_same_v<T, std::string>) {
            std::ifstream is = file.getInputStream();
            if (!is.is_open()) {
                std::cerr << "Error: Could not open file for reading: " << file.path_ << std::endl;
                return file;
            }

            try {
                std::stringstream buffer;
                buffer << is.rdbuf();
                data = static_cast<std::string&>(data) = buffer.str();
                file.success_ = true;
            } catch (const std::exception& e) {
                std::cerr << "Error during file reading: " << e.what() << std::endl;
            }
        } else {
            std::ifstream is = file.getInputStream();

            if (!is.is_open()) {
                std::cerr << "Error: Could not open file for reading: " << file.path_ << std::endl;
                return file;
            }

            if (is >> data) {
                file.success_ = true;
            } else {
                std::cerr << "Error: Failed to extract token of specific type." << std::endl;
            }
        }

        return file;
    }
}