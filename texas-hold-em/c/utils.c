#include <conio.h>
#include <processenv.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <winnt.h>
#include <ws2tcpip.h>

#include "utils.h"

int uul_server_Initialize(SOCKET *client_socket) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return 0;
    }
    printf("client_socket ptr = %p\n", (void *)client_socket);
    *client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    printf("Here");
    if (*client_socket == INVALID_SOCKET) {
        WSACleanup();
        return 0;
    }

    char broadcast_on = 1;
    if (setsockopt(*client_socket, SOL_SOCKET, SO_BROADCAST, &broadcast_on, sizeof(broadcast_on)) == SOCKET_ERROR) {
        closesocket(*client_socket);
        WSACleanup();
        return 0;
    }

    return 1;
}

void uul_server_Cleanup(SOCKET *sockets, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        if (sockets[i] != INVALID_SOCKET) {
            closesocket(sockets[i]);
            sockets[i] = INVALID_SOCKET;
        }
    }

    WSACleanup();

    printf("Winsock cleanup complete.\n");
}

int uul_server_CreateServer(int port, SOCKET *listen_socket) {
    if (*listen_socket != INVALID_SOCKET) {
        fprintf(stderr, "Server is already listening: %d\n", WSAGetLastError());
        return 0;
    }

    *listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (*listen_socket == INVALID_SOCKET) {
        fprintf(stderr, "Error creating socket: %d\n", WSAGetLastError());
        return 0;
    }

    struct sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = htonl(INADDR_ANY);
    service.sin_port = htons(port);

    if (bind(*listen_socket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
        fprintf(stderr, "Bind failed with error: %d\n", WSAGetLastError());
        closesocket(*listen_socket);
        *listen_socket = INVALID_SOCKET;
        return 0;
    }

    if (listen(*listen_socket, SOMAXCONN) == SOCKET_ERROR) {
        fprintf(stderr, "Listen failed with error: %d\n", WSAGetLastError());
        closesocket(*listen_socket);
        *listen_socket = INVALID_SOCKET;
        return 0;
    }

    printf("Server successfully started and listening on port %d.\n", port);
    return 1;
}

SOCKET uul_server_AcceptClient(SOCKET listen_socket) {
    SOCKET accepted_client_socket = INVALID_SOCKET;
    struct sockaddr_in client_info;
    int client_info_size = sizeof(client_info);

    if (listen_socket == INVALID_SOCKET) {
        fprintf(stderr, "Error: Cannot accept client. Listening socket is not active.\n");
        return INVALID_SOCKET;
    }

    printf("Waiting for a new client connection...\n");

    accepted_client_socket = accept(listen_socket, (SOCKADDR*)&client_info, &client_info_size);

    if (accepted_client_socket == INVALID_SOCKET) {
        fprintf(stderr, "Accept failed with error: %d\n", WSAGetLastError());
        return INVALID_SOCKET;
    }

    char client_ip[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &(client_info.sin_addr), client_ip, INET_ADDRSTRLEN)) {
        printf("Client accepted from IP: %s\n", client_ip);
    }

    return accepted_client_socket;
}

int uul_server_ConnectClient(const char *ip, int port, SOCKET *client_socket) {
    if (*client_socket != INVALID_SOCKET) {
        closesocket(*client_socket);
        *client_socket = INVALID_SOCKET;
    }

    *client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (*client_socket == INVALID_SOCKET) {
        fprintf(stderr, "Error creating client socket: %d\n", WSAGetLastError());
        return 0;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &(server_addr.sin_addr)) != 1) {
        fprintf(stderr, "Error converting IP address: %s\n", *ip);
        closesocket(*client_socket);
        *client_socket = INVALID_SOCKET;
        return 0;
    }

    printf("Attempting to connect to %s:%d...\n", ip, port);

    if (connect(*client_socket, (SOCKADDR*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        fprintf(stderr, "Connection failed with error: %d\n", WSAGetLastError());
        closesocket(*client_socket);
        *client_socket = INVALID_SOCKET;
        return 0;
    }

    printf("Successfully connected to game server!\n");
    return 1;
}

int push_ip(char ***list_ptr, size_t *count, const char *ip) {
    char **tmp = realloc(*list_ptr, (*count + 1) * sizeof(char*));
    if (!tmp) return 0;

    *list_ptr = tmp;
    (*list_ptr)[*count] = _strdup(ip);
    if (!(*list_ptr)[*count]) return 0;

    (*count)++;
    return 1;
}

char **uul_server_SearchForLANGames(SOCKET *client_socket) {
    char **active_ips;
    size_t active_ip_count = 0;

    struct sockaddr_in broadcast_addr;
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(uul_server_defaultport);
    broadcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

    const char *query_message = "LAN_GAME_SEARCH";
    if (sendto(*client_socket, query_message, (int)strlen(query_message), 0, (SOCKADDR*)&broadcast_addr, sizeof(broadcast_addr)) == SOCKET_ERROR) {
        return active_ips;
    }

    DWORD timeout_ms = 5000;
    setsockopt(*client_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout_ms, sizeof(timeout_ms));

    char recv_buf[512];
    struct sockaddr_in server_addr;
    int server_addr_size = sizeof(server_addr);

    while (1) {
        int bytes_received = recvfrom(*client_socket, recv_buf, sizeof(recv_buf), 0, (SOCKADDR*)&server_addr, &server_addr_size);

        if (bytes_received > 0) {
            char ip_str[INET_ADDRSTRLEN];
            if (inet_ntop(AF_INET, &(server_addr.sin_addr), ip_str, INET_ADDRSTRLEN)) {
                push_ip(&active_ips, &active_ip_count, ip_str);
            }
        } else if (bytes_received == 0) {
            break;
        } else {
            if (WSAGetLastError() == WSAETIMEDOUT) {
                break;
            } else {
                break;
            }
        }
    }

    return active_ips;
}

int uul_server_StartDiscoveryListener(int port, SOCKET *udp_discovery_socket) {
    int *test;

    if (*udp_discovery_socket != INVALID_SOCKET) {
        fprintf(stderr, "UDP listener already running or socket not cleaned up,\n");
        return 0;
    }

    *udp_discovery_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (*udp_discovery_socket == INVALID_SOCKET) {
        fprintf(stderr, "Error creating UDP discovery socket: %d\n", WSAGetLastError());
        return 0;
    }

    struct sockaddr_in host_addr;
    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(port);
    host_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(*udp_discovery_socket, (SOCKADDR*)&host_addr, sizeof(host_addr)) == SOCKET_ERROR) {
        fprintf(stderr, "UDP bind failed with error: %d\n", WSAGetLastError());
        closesocket(*udp_discovery_socket);
        *udp_discovery_socket = INVALID_SOCKET;
        return 0;
    }

    printf("UDP discovery listener started on port %d.\n", port);
    return 0;
}

void uul_server_RunDiscoverLoop(SOCKET udp_discovery_socket) {
    if (udp_discovery_socket == INVALID_SOCKET) {
        fprintf(stderr, "Cannot run discover loop: Socket is invalid.\n");
        return;
    }

    char recv_buf[512];
    struct sockaddr_in client_addr;
    int client_addr_size = sizeof(client_addr);
    const char *SERVER_INFO = "My Ultra Game Server";

    printf("UDP discovery loop is active.\n");

    while (1) {
        int bytes_received = recvfrom(udp_discovery_socket, recv_buf, sizeof(recv_buf), 0, (SOCKADDR*)&client_addr, &client_addr_size);

        if (bytes_received > 0) {
            recv_buf[bytes_received] = '\0';

            if (strcmp(recv_buf, "LAN_GAME_SEARCH") == 0) {
                sendto(udp_discovery_socket, SERVER_INFO, (int)strlen(SERVER_INFO), 0, (SOCKADDR*)&client_addr, client_addr_size);
            }
        } else if (bytes_received = SOCKET_ERROR) {
            int error = WSAGetLastError();
            if (error == WSAEINTR || error == WSAENOTSOCK) {
                break;
            } else {
                fprintf(stderr, "Discovery listener error: %d\n", error);
            }
        }
    }
}

int uul_server_SendRequest(SOCKET s, const char *message) {
    if (s == INVALID_SOCKET) return 0;

    uint32_t length = (uint32_t)strlen(message);
    uint32_t net_length = htonl(length);

    int bytes_sent = send(s, (const char*)(&net_length), sizeof(net_length), 0);
    if (bytes_sent == SOCKET_ERROR) {
        fprintf(stderr, "Failed to send message length: %d\n", WSAGetLastError());
        return 0;
    }

    if (length > 0) {
        bytes_sent = send(s, message, length, 0);
        if (bytes_sent == SOCKET_ERROR) {
            fprintf(stderr, "Failed to send message body: %d\n", WSAGetLastError());
            return 0;
        }
    }

    return 1;
}

char *uul_server_ReceiveRequest(SOCKET s) {
    if (s == INVALID_SOCKET) return "";

    uint32_t net_length = 0;
    int bytes_received = recv(s, (char*)&net_length, sizeof(net_length), 0);

    if (bytes_received <= 0) {
        return "";
    }

    uint32_t length = ntohl(net_length);
    if (length == 0) return "";

    char *message = malloc(length);

    uint32_t total_received = 0;
    while (total_received < length) {
        bytes_received = recv(s, &message[total_received], length - total_received, 0);
        if (bytes_received <= 0) {
            fprintf(stderr, "Error receiving message body or connection closed.\n");
            return "";
        }
        total_received += bytes_received;
    }
    
    return message;
}

void uul_cli_goToXY(HANDLE hConsole, int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(hConsole, coord);
}

COORD uul_cli_getCursorPosition(HANDLE hConsole) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        return csbi.dwCursorPosition;
    }
    return (COORD){0,0};
}

void uul_cli_EnableUTF8() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
}

void uul_cli_render(HANDLE hConsole, COORD startPos, int selected, const char *options[], int options_size, const char question[]) {
    uul_cli_goToXY(hConsole, 0, startPos.Y);
    printf("\x1b[2K%s\n", question);

    for (size_t i = 0; i < options_size; ++i) {
        printf("\x1b[2K");
        const char *prefix = (selected == i) ? "\x1b[36m> \x1b[0m" : "  ";
        printf("%s%s\n", prefix, options[i]);
    }
    fflush(stdout);
}

const char *uul_cli_MultipleChoice(HANDLE hConsole, const char *options[], int options_size, const char question[]) {
    if (options_size <= 0) return NULL;

    int selected = 0;
    COORD startPos = uul_cli_getCursorPosition(hConsole);

    uul_cli_render(hConsole, startPos, selected, options, options_size, question);

    while (1) {
        int key = _getch();
        if (key == 0 || key == 0xE0) {
            key = _getch();
            if (key == 72) {
                selected = (selected - 1 + options_size) % options_size;
            } else if (key == 80) {
                selected = (selected + 1) % options_size;
            }
            uul_cli_render(hConsole, startPos, selected, options, options_size, question);
        } else if (key == ' ' || key == '\n' || key == '\r') {
            break;
        } else if (key == 3) {
            exit(0);
        }
    }

    for (int i = 0; i < options_size; ++i) {
        uul_cli_goToXY(hConsole, 0, startPos.Y + 1);
        printf("\x1b[2K");
    }
    uul_cli_goToXY(hConsole, 0, startPos.Y);

    return options[selected];
}

char *read_file(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char *buffer = malloc(size + 1);
    if (!buffer) {
        fclose(f);
        return NULL;
    }

    fread(buffer, 1, size, f);
    buffer[size] = '\0';

    fclose(f);
    return buffer;
}

int write_file(const char *filename, const char *data) {
    FILE *f = fopen(filename, "wb");
    if (!f) return 0;

    fputs(data, f);
    fclose(f);
    return 1;
}