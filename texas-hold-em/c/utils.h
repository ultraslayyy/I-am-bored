#include <winnt.h>
#include <winsock2.h>

static const int uul_server_defaultport = 54000;

#define BCAST_FMT(fmt, ...) do { \
        char _msg[128]; \
        snprintf(_msg, sizeof(_msg), fmt, __VA_ARGS__); \
        broadcastRequest(players, player_count, _msg); \
    } while (0)

/**
 * @brief Initializes the Winsock library (called first).
 * @return true on success, false on failure.
 */
int uul_server_Initialize(SOCKET *client_socket);

/**
 * @brief Cleans up Winsock library (called last).
 * @param sockets List of sockets to cleanup.
 * @param count Number of sockets being cleaned up.
 */
void uul_server_Cleanup(SOCKET *sockets, size_t count);

/**
 * @brief Creates a simple TCP server and starts listening.
 * @param port The port to listen on.
 * @param listen_socket The listener socket.
 * @return 1 if the server is successfully listening, 0 otherwise.
 */
int uul_server_CreateServer(int port, SOCKET *listen_socket);

/**
 * @brief Waits for and accepts a client connection (blocking).
 * @param listen_socket The listener socket.
 * @return The client socket handle on success, or INVALID_SOCKET on failure.
 */
SOCKET uul_server_AcceptClient(SOCKET listen_socket);

/**
 * @brief Connects to a server.
 * @param ip The server's IP address (e.g., "127.0.0.1" for localhost).
 * @param port The server's port.
 * @param client_socket The socket of the client.
 * @return 1 if successful connection, 0 otherwise.
 */
int uul_server_ConnectClient(const char *ip, int port, SOCKET *client_socket);

/**
 * @brief Searches for LAN games.
 * @param client_socket The socket to search with.
 * @return An array of ip's running games.
 */
char** uul_server_SearchForLANGames(SOCKET *client_socket);

/**
 * @brief Starts the UDP listener for discovery.
 * @param port Port to listen on.
 * @param udp_discovery_socket The socket to run the listener on.
 * @return 1 if successful, 0 otherwise.
 */
int uul_server_StartDiscoveryListener(int port, SOCKET *udp_discovery_socket);

/**
 * @brief Run the loop to run discovery
 * @param udp_discovery_socket The socket to run the loop for (must be discovery listener).
 */
void uul_server_RunDiscoverLoop(SOCKET udp_discovery_socket);

/**
 * @brief Sends a string message over a specific socket.
 * @param s Socket to send the message with.
 * @param message The message to send.
 * @return 1 if successful, 0 otherwise
 */
int uul_server_SendRequest(SOCKET s, const char *message);

/**
 * @brief Receives a string message from a specific socket (blocking).
 * @param s Socket to receive the message for.
 * @return Received message or "" if failure.
 */
char* uul_server_ReceiveRequest(SOCKET s);


/**
 * @brief Move caret to (X, Y).
 * @param hConsole Console handle.
 * @param x X value.
 * @param y Y value.
 */
void uul_cli_goToXY(HANDLE hConsole, int x, int y);

/**
 * @brief Get the current position of the caret.
 * @return The position of the caret as a COORD (.X, .Y).
 */
COORD uul_cli_getCursorPosition(HANDLE hConsole);

/**
 * @brief Enables UTF-8 characters in terminal.
 */
void uul_cli_EnableUTF8();

/**
 * @brief Creates a multiple choice question with arrow key navigation.
 * @param hConsole Console handle.
 * @param options An array of options.
 * @param options_size The number of options.
 * @param question The question being asked.
 * @return The option selected.
 */
const char* uul_cli_MultipleChoice(HANDLE hConsole, const char *options[], int options_size, const char question[]);


/**
 * @brief Read a file from a path.
 * @param filename The name of the file.
 * @return The file data.
 */
char *read_file(const char *filename);

/**
 * @brief Write to a file.
 * @param path The name of the file.
 * @param data The data to write.
 * @return 1 if successful, 0 otherwise.
 */
int write_file(const char *filename, const char *data);