#include <conio.h>
#include <inaddr.h>
#include <processenv.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <winbase.h>
#include <winnt.h>

#include "utils.h"
#include "json/cJSON.h"

#define CLEAR_SCREEN_CMD "cls"
#define PLAYER_NAME_LEN 33

HANDLE hConsole;
SOCKET clientSocket = INVALID_SOCKET;

void enterIpMenu();
void joinMenu();
void main_menu();
void settingsMenu();
void clientGame(SOCKET cSocket);

typedef struct {
    int port;
    SOCKET udp_discovery_socket;
} DiscoveryThreadArgs;

typedef struct {
    SOCKET socket_handle;

    char name[PLAYER_NAME_LEN];
    int chips;
    int pIndex;

    char hand[2][8];
    int folded;
    int allIn;
    int roundBet;
} Player;

void player_init(Player *p, SOCKET s) {
    p->socket_handle = s;

    strncpy(p->name, "Guest", PLAYER_NAME_LEN);
    p->chips = 0;
    p->pIndex = -1;

    for (size_t i = 0; i < 2; ++i) {
        p->hand[i][0] = '\n';
    }

    p->folded = 0;
    p->allIn = 0;
    p->roundBet = 0;
}

void player_cleanup(Player *p) {
    if (p->socket_handle != INVALID_SOCKET) {
        printf("Closing socket for player.\n");
        closesocket(p->socket_handle);
        p->socket_handle = INVALID_SOCKET;
    }
}

void show_windows_error_dialog(const wchar_t* title, const wchar_t* message) {
    int result = MessageBoxW(
        NULL,
        message,
        title,
        MB_ICONERROR | MB_OK
    );
}

typedef struct {
    char name[33];
} ClientSettings;

typedef struct {
    int smallBlind;
    int bigBlind;
    int players;
    int startingChips;
} ServerSettings;

ClientSettings clientSettings = {
    .name = "Guest"
};

ServerSettings serverSettings = {
    .smallBlind = 10,
    .bigBlind = 20,
    .players = 4,
    .startingChips = 1000
};

#pragma region JSON

int saveSettings(const char *filename) {
    cJSON *root = cJSON_CreateObject();
    cJSON *server = cJSON_CreateObject();
    cJSON *client = cJSON_CreateObject();

    cJSON_AddNumberToObject(server, "smallBlind", serverSettings.smallBlind);
    cJSON_AddNumberToObject(server, "bigBlind", serverSettings.bigBlind);
    cJSON_AddNumberToObject(server, "players", serverSettings.players);
    cJSON_AddNumberToObject(server, "startingChips", serverSettings.startingChips);

    cJSON_AddStringToObject(client, "name", clientSettings.name);

    cJSON_AddItemToObject(root, "server", server);
    cJSON_AddItemToObject(root, "client", client);

    char *json = cJSON_Print(root);
    int ok = write_file(filename, json);

    cJSON_Delete(root);
    free(json);

    return ok;
}

int loadSettings(const char *filename, char *error, size_t errorSize) {
    char *text = read_file(filename);
    if (!text) {
        snprintf(error, errorSize, "Could not open file");
        return 0;
    }

    cJSON *root = cJSON_Parse(text);
    free(text);

    if (!root) {
        snprintf(error, errorSize, "Invalid JSON");
        return 0;
    }

    cJSON *server = cJSON_GetObjectItem(root, "server");
    cJSON *client = cJSON_GetObjectItem(root, "client");

    if (!cJSON_IsObject(server) || !cJSON_IsObject(client)) {
        snprintf(error, errorSize, "Missing 'server' or 'client' object");
        cJSON_Delete(root);
        return 0;
    }

    cJSON *v;
    if ((v = cJSON_GetObjectItem(server, "smallBlind"))) serverSettings.smallBlind = v->valueint;
    if ((v = cJSON_GetObjectItem(server, "bigBlind")))   serverSettings.bigBlind   = v->valueint;
    if ((v = cJSON_GetObjectItem(server, "players")))    serverSettings.players    = v->valueint;
    if ((v = cJSON_GetObjectItem(server, "startingChips"))) serverSettings.startingChips = v->valueint;

    v = cJSON_GetObjectItem(client, "name");
    if (cJSON_IsString(v)) {
        strncpy(clientSettings.name, v->valuestring, sizeof(clientSettings.name) - 1);
        clientSettings.name[sizeof(clientSettings.name) - 1] = '\0';
    }

    cJSON_Delete(root);
    return 1;
}


#pragma endregion JSON

void string_erase(char *str, size_t pos, size_t len) {
    size_t str_len = strlen(str);
    
    if (pos >= str_len) return;
    
    if (pos + len >= str_len) {
        str[pos] = '\0';
    } else {
        memmove(str + pos, str + pos + len, str_len - pos - len + 1);
    }
}

void string_insert(char *str, size_t pos, const char * insertStr) {
    size_t str_len = strlen(str);
    size_t insert_len = strlen(insertStr);

    if (pos > str_len) pos = str_len;

    memmove(str + pos + insert_len, str + pos, str_len - pos + 1); 
    memcpy(str + pos, insertStr, insert_len);
}

#pragma region Game

char deck[52][8];
int deck_count = 0;

void shuffle(char arr[][8], int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);

        // Fisher-Yates shuffle
        char temp[8];
        memcpy(temp, arr[i], 8);
        memcpy(arr[i], arr[j], 8);
        memcpy(arr[j], temp, 8);
    }
}

void shuffle_int(int *arr, size_t n) {
    if (n <= 1) return;

    for (size_t i = n - 1; i > 0; i--) {
        size_t j = rand() % (i + 1);
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

void genDeck() {
    const char *suits[4] = {"♥", "♠", "♦", "♣"};
    const char *ranks[13] = {"A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"};

    deck_count = 0;

    for (int s = 0; s < 4; s++) {
        for (int r = 0; r < 13; r++) {
            snprintf(deck[deck_count], sizeof(deck[deck_count]), "%s%s", ranks[r], suits[s]);
        }
    }

    shuffle(deck, 52);
}

char *genCard() {
    if (deck_count == 0) {
        fprintf(stderr, "Deck is empty");
        return NULL;
    }
    deck_count--;
    return deck[deck_count];
}

void broadcastRequest(Player **players, int player_count, char *msg) {
    for (size_t i = 0; i < player_count; ++i) {
        uul_server_SendRequest(players[i]->socket_handle, msg);
    }
}

void bettingRound(Player **players, size_t player_count, int *pot, const char *phase) {
    int currentBet = (strcmp(phase, "PRE_FLOP") == 0) ? serverSettings.bigBlind : 0;
    size_t lastRaiserIndex = (strcmp(phase, "PRE_FLOP") == 0) ? 1 : player_count;
    size_t startingPlayerIndex;

    if (strcmp(phase, "PRE_FLOP") == 0) {
        startingPlayerIndex = 3 % player_count;
        lastRaiserIndex = 2;
    } else {
        startingPlayerIndex = 1;
        lastRaiserIndex = player_count;
    }

    for (size_t i = 0; i < player_count; ++i) {
        Player *p = players[i];
        if (!p->folded) {
            if (strcmp(phase, "PRE_FLOP") == 0) {
                if (p->pIndex == 1) p->roundBet = serverSettings.smallBlind;
                else if (p->pIndex == 2) p->roundBet = serverSettings.bigBlind;
                else p->roundBet = 0;
            } else {
                p->roundBet = 0;
            }
        }
    }

    size_t currentPlayerIndex = startingPlayerIndex;

    do {
        Player *currentPlayer = NULL;
        for (size_t i = 0; i < player_count; ++i) {
            Player *p = players[i];
            if (p->pIndex == currentPlayerIndex) {
                currentPlayer = p;
                break;
            }
        }

        if (!currentPlayer || currentPlayer->folded || (currentPlayer->allIn && currentPlayer->roundBet >= currentBet)) {
            currentPlayerIndex = (currentPlayerIndex + 1) % player_count;
            continue;
        }

        int maxRoundBet = 0;
        int activePlayersCount = 0;
        int allActivePlayersSatisfied = 1;

        for (size_t i = 0; i < player_count; ++i) {
            Player *p = players[i];
            if (!p->folded) {
                activePlayersCount++;
                if (p->roundBet > maxRoundBet) {
                    maxRoundBet = p->roundBet;
                }

                if (!p->allIn && p->roundBet < currentBet) {
                    allActivePlayersSatisfied = 0;
                }
            }
        }
        currentBet = maxRoundBet;

        if (activePlayersCount <= 1) {
            broadcastRequest(players, player_count, "HAND_OVER:ONLY_ONE_PLAYER");
            return;
        }

        if (allActivePlayersSatisfied && currentPlayerIndex == lastRaiserIndex) {
            break;
        }

        /*
        if (currentPlayer->roundBet >= currentBet && currentPlayerIndex != lastRaiserIndex) {
            currentPlayerIndex = (currentPlayerIndex + 1) & player_count;
            continue;
        } */
        currentPlayerIndex = (currentPlayerIndex + 1) & player_count;

        int amountToCall = currentBet - currentPlayer->roundBet;

        char requestMsg[256];
        snprintf(
            requestMsg,
            sizeof(requestMsg),
            "YOUR_TURN:CALL:%d|MIN_RAISE:%d|CHIPS:%d",
            amountToCall,
            currentBet + amountToCall,
            currentPlayer->chips
        );

        uul_server_SendRequest(currentPlayer->socket_handle, requestMsg);

        char *actionMsg = uul_server_ReceiveRequest(currentPlayer->socket_handle);
        char *action = (char *)malloc(strlen(actionMsg) * sizeof(char));
        if (action == NULL) continue;
        const char *space = strchr(actionMsg, ' ');
        size_t len = space ? (size_t)(space - actionMsg) : strlen(actionMsg);
        memcpy(action, actionMsg, len);
        action[len] = '\0';

        int amount = 0;
        if (space) amount = (int)strtol(space + 1, NULL, 10);

        int amountComitted = 0;

        char bmsg[128];
        snprintf(bmsg, sizeof(bmsg), "ACTION:%s", currentPlayer->name);
        
        if (strcmp(action, "FOLD") == 0) {
            currentPlayer->folded = 1;
            char msg[128];
            snprintf(msg, sizeof(msg), "%s:FOLD", bmsg);
            broadcastRequest(players, player_count, msg);
        } else if (strcmp(action, "CALL") == 0 || (strcmp(action, "RAISE") == 0 && amount < currentBet)) {
            amountComitted = amountToCall;
            char msg[128];
            snprintf(msg, sizeof(msg), "%s:CALL %d", bmsg, amountToCall);
            broadcastRequest(players, player_count, msg);
        } else if (strcmp(action, "RAISE") == 0) {
            amountComitted = amount - currentPlayer->roundBet;

            if (amount > currentBet && amountComitted <= currentPlayer->chips) {
                if (amount - currentBet < (currentBet - (currentBet - amountToCall))) {
                    // TODO: fix
                }

                currentBet = amount;
                lastRaiserIndex = currentPlayerIndex;
                char msg[128];
                snprintf(msg, sizeof(msg), "%s:RAISE to  %d", bmsg, amount);
                broadcastRequest(players, player_count, msg);
            } else {
                amountComitted = amountToCall;
                char msg[128];
                snprintf(msg, sizeof(msg), "%s:CALL (Invalid RAISE)", bmsg);
                broadcastRequest(players, player_count, msg);
            }
        }

        free(action);

        if (amountComitted > 0) {
            int actualCommit = min(amountComitted, currentPlayer->chips);

            currentPlayer->chips -= actualCommit;
            currentPlayer->roundBet += actualCommit;
            *pot += actualCommit;

            if (currentPlayer->chips == 0) {
                currentPlayer->allIn = 1;
                char msg[128];
                snprintf(msg, sizeof(msg), "%s:ALL-IN!", bmsg);
                broadcastRequest(players, player_count, msg);
            }
        }

        currentPlayerIndex = (currentPlayerIndex + 1) % player_count;
    } while (1);

    BCAST_FMT("END_BETTING_ROUND:POT_SIZE:", *pot);
}

void runBlinds(Player **players, size_t player_count, int *pot) {
    for (size_t i = 0; i < player_count; ++i) {
        Player *p = players[i];

        if (p->pIndex == 1) {
            p->chips -= serverSettings.smallBlind;
            broadcastRequest(players, player_count, strcat("small_blind:", p->name));
        } else if (p->pIndex == 2) {
            p->chips -= serverSettings.bigBlind;
            broadcastRequest(players, player_count, strcat("big_blind:", p->name));
        }
    }

    *pot += 10;
    *pot += 20;
}

void randomisePlayerOrder(Player **players, size_t player_count) {
    int *indices = (int*)calloc(player_count, sizeof(int));
    if (!indices) return;
    for (size_t i = 0; i < player_count; ++i) indices[i] = (int)i;

    shuffle_int(indices, player_count);
    for (size_t i = 0; i < player_count; ++i) players[i]->pIndex = indices[i];
    
    free(indices);
}

void serverGame(Player **players, size_t player_count) {
    genDeck();

    int *pot = 0;
    char comCards[5][8];

    randomisePlayerOrder(players, player_count);
    runBlinds(players, player_count, pot);

    for (size_t i = 0; i < player_count; ++i) {
        Player *p = players[i];
        strncpy(p->hand[0], genCard(), sizeof(p->hand[0]));
        strncpy(p->hand[1], genCard(), sizeof(p->hand[0]));
        
        char handStr[18];
        snprintf(handStr, sizeof(handStr), "%s %s", p->hand[0], p->hand[1]);
        uul_server_SendRequest(p->socket_handle, handStr);
    }
    broadcastRequest(players, player_count, "HOLE_CARDS_DEALT");
    BCAST_FMT("PHASE:PRE_FLOP|POT:", *pot);
    bettingRound(players, player_count, pot, "PRE_FLOP");

    strncpy(comCards[0], genCard(), sizeof(comCards[0]));
    strncpy(comCards[1], genCard(), sizeof(comCards[0]));
    strncpy(comCards[2], genCard(), sizeof(comCards[0]));
    char flopStr[50];
    snprintf(
        flopStr,
        sizeof(flopStr),
        "COMMUNITY_CARDS:%s %s %s",
        comCards[0],
        comCards[1],
        comCards[2]
    );

    BCAST_FMT("PHASE:FLOP|POT:", *pot);
    broadcastRequest(players, player_count, flopStr);
    bettingRound(players, player_count, pot, "POST_FLOP");

    strncpy(comCards[3], genCard(), sizeof(comCards[0]));
    char *turnStr = strcat(strcat(flopStr, " "), comCards[3]);

    BCAST_FMT("PHASE:TURN|POT:", *pot);
    broadcastRequest(players, player_count, turnStr);
    bettingRound(players, player_count, pot, "POST_FLOP");

    strncpy(comCards[4], genCard(), sizeof(comCards[0]));
    char *riverStr = strcat(strcat(turnStr, " "), comCards[4]);

    BCAST_FMT("PHASE:RIVER|POT:", *pot);
    broadcastRequest(players, player_count, riverStr);
    bettingRound(players, player_count, pot, "POST_FLOP");

    BCAST_FMT("PHASE:SHOWDOWN|POT:", *pot);
}

void RunDiscoveryListener(SOCKET s) {
    printf("Starting Discovery Listener thread...\n");
    uul_server_RunDiscoverLoop(s);
    printf("Discovery Listener thread terminated!\n");
}

DWORD WINAPI start_discovery_thread(LPVOID arg) {
    DiscoveryThreadArgs *args = (DiscoveryThreadArgs*)arg;

    if (!uul_server_StartDiscoveryListener(args->port, &args->udp_discovery_socket)) {
        fprintf(stderr, "FATAL: Failed to start UDP Discovery Listener.\n");
        free(args);
        main_menu();
        return 1;
    }

    uul_server_RunDiscoverLoop(args->udp_discovery_socket);

    free(args);
    return 0;
}

void createGame() {
    system(CLEAR_SCREEN_CMD);
    printf("Attempting to create a new game server...\n");

    DiscoveryThreadArgs *args = malloc(sizeof(DiscoveryThreadArgs));
    if (args) {
        args->port = uul_server_defaultport;
        args->udp_discovery_socket = INVALID_SOCKET;

        HANDLE discovery_thread = CreateThread(
            NULL,
            0,
            start_discovery_thread,
            args,
            0,
            NULL
        );

        if (!discovery_thread) {
            fprintf(stderr, "Failed to created discovery thread\n");
            free(args);
        }

        CloseHandle(discovery_thread);
    }

    SOCKET listen_socket = INVALID_SOCKET;

    if (!uul_server_CreateServer(uul_server_defaultport, &listen_socket)) {
        fprintf(stderr, "FATAL: Failed to start TCP Game Server. Check binding/port.\n");
        uul_server_Cleanup((SOCKET []){listen_socket}, 1); // Program is ending. No need to clean udp
        return;
    }

    const int MAX_CLIENTS = 5;
    Player **connected_players = calloc(MAX_CLIENTS, sizeof(Player*));
    if (!connected_players) return;
    size_t player_count = 0;

    while (player_count < MAX_CLIENTS) {
        SOCKET new_client_socket = uul_server_AcceptClient(listen_socket);

        if (new_client_socket != INVALID_SOCKET) {
            char *msg = uul_server_ReceiveRequest(new_client_socket);
            char playerName[33];
            strncpy(playerName, msg, sizeof(playerName) - 1);
            playerName[32] = '\0';
            free(msg);

            Player *new_player = malloc(sizeof(Player));
            if (!new_player) continue;
            player_init(new_player, new_client_socket);
            strncpy(new_player->name, playerName, sizeof(new_player->name) - 1);
            new_player->chips = serverSettings.startingChips;
            connected_players[player_count++] = new_player;

            size_t existing_player_count = player_count - 1;
            Player **existingPlayers = calloc(existing_player_count, sizeof(Player *));
            if (!existingPlayers) continue;
            for (size_t i = 0; i < existing_player_count - 1; ++i) {
                existingPlayers[i] = connected_players[i];
            }

            broadcastRequest(existingPlayers, existing_player_count, strcat("player connected:", connected_players[player_count]->name));
            free(existingPlayers);
        } else {
            fprintf(stderr, "Error accepting client. Exiting server loop.\n");
            break;
        }

        broadcastRequest(connected_players, player_count, "game_start");

        serverGame(connected_players, player_count);

        uul_server_Cleanup((SOCKET []){listen_socket}, 1);
        for (size_t i = 0; i < player_count; ++i) {
            player_cleanup(connected_players[i]);
            free(connected_players[i]);
        }
        free(connected_players);
    }
}

void clientJoinGame(char *ip) {
    if (uul_server_ConnectClient(ip, uul_server_defaultport, &clientSocket)) {
        char name[33] = "Guest";

        uul_server_SendRequest(clientSocket, name);

        const char *serverRequestPrefixes[2] = {
            "played_connected:",
            "game_start"
        };

        int waiting = 1;
        while (waiting) {
            char *msg = uul_server_ReceiveRequest(clientSocket);
            
            for (size_t i = 0; i < 2; ++i) {
                if (strncmp(msg, serverRequestPrefixes[i], strlen(serverRequestPrefixes[i])) == 0) {
                    switch (i) {
                        case 0:
                            char *cPos = strchr(msg, ':');
                            if (cPos != NULL) {
                                cPos++;
                                printf("Player connected: %s\n", cPos);
                            }
                            break;
                        case 1:
                            free(msg);
                            clientGame(clientSocket);
                            break;
                    }
                    break;
                }
            }
        }

        clientGame(clientSocket);
    } else {
        fprintf(stderr, "Failed to connected to %s. Returning to join menu.\n", ip);
        joinMenu();
    }
}

size_t find_char_from(const char *str, char c, size_t start) {
    const char *pos= strchr(str + start, c);
    return pos ? (size_t)(pos - str) : SIZE_MAX;
}

int extractValue(const char *msg, const char *key) {
    const char *keyPosPtr = strstr(msg, key);
    if (!keyPosPtr) return 0;

    size_t keyPos = keyPosPtr - msg;
    size_t startPos = keyPos + strlen(key);

    size_t endPos = find_char_from(msg, '|', startPos);
    if (endPos == SIZE_MAX) endPos = strlen(msg);

    size_t valueLen = endPos - startPos;
    char *valueStr = (char *)malloc(valueLen + 1);
    if (!valueStr) return 0;
    strncpy(valueStr, msg + startPos, valueLen);
    valueStr[valueLen] = '\0';

    int value = atoi(valueStr);
    free(valueStr);

    return value;
}

void clientGame(SOCKET cSocket) {
    system(CLEAR_SCREEN_CMD);
    printf("Game started! Waiting for action...\n");

    const char *serverRequestPrefixes[9] = {
        "small_blind:",
        "big_blind:",
        "COMMUNITY_CARDS:",
        "PHASE:",
        "YOUR_TURN:",
        "ACTION",
        "HOLE_CARDS_DEALT:",
        "END_BETTING_ROUND:",
        "HAND_OVER"
    };

    int gameRunning = 1;
    char *playerHand = "";
    char *communityCards = "";
    int currentPot = 0;

    while (gameRunning) {
        char *msg = uul_server_ReceiveRequest(cSocket);

        int has_suit = strstr(msg, "♥") || strstr(msg, "♠") || strstr(msg, "♦") || strstr(msg, "♣");
        if (has_suit && strlen(msg) < 10 && strchr(msg, ':') == NULL) {
            playerHand = msg;
            printf("\n** Your Hand: %s **\n", playerHand);
            continue;
        }

        int handled = 0;
        for (size_t i= 0; i < (sizeof(serverRequestPrefixes) / sizeof(serverRequestPrefixes[0])); ++i) {
            if (strncmp(msg, serverRequestPrefixes[i], sizeof(serverRequestPrefixes[i])) == 0) {
                switch (i) {
                    case 0:
                    case 1: {
                        const char *name = strchr(msg, ':');
                        if (name) name++;
                        printf("BLIND: %s posts a blind.\n", name);
                        break;
                    }
                    case 2:
                        communityCards = strchr(msg, ':');
                        if (communityCards) communityCards++;
                        printf("\n*** COMMUNITY CARDS: %s ***\n", communityCards);
                        break;
                    case 3: {
                        size_t start = find_char_from(msg, ':', 0);
                        size_t end = find_char_from(msg, '|', start + 1);

                        if (start != SIZE_MAX && end != SIZE_MAX && end > start) {
                            size_t len = end - start - 1;
                            char phaseName[128];
                            memcpy(phaseName, msg + start + 1, len);
                            phaseName[len] = '\0';
                            
                            currentPot = extractValue(msg, "POT:");
                            printf("\n--- Starting %s Phase (Pot: %d) ---\n", phaseName, currentPot);
                        }
                        break;
                    }
                    case 4: {
                        int callAmount = extractValue(msg, "CALL:");
                        int minRaise = extractValue(msg, "MIN_RAISE:");
                        int chips = extractValue(msg, "CHIPS:");

                        printf("\n--- IT IS YOUR TURN! ---\n");
                        printf("Your Hand: %s | Community: %s | Pot: %d\n", playerHand, communityCards, currentPot);
                        printf("Chips: %d\n", chips);

                        char action[10];
                        do {
                            printf("Enter action (FOLD, CALL, (%d), RAISE [>%d]): ", callAmount, minRaise);
                            fgets(action, sizeof(action), stdin);
                        } while (action[0] == '\0');

                        uul_server_SendRequest(cSocket, action);
                        break;
                    }
                    case 5: {
                        const char *action = strchr(msg, ':');
                        if (action) action++;
                        printf("Player Action: %s\n", action);
                        break;
                    }
                    case 6:
                        printf("Hole cards are dealt to all players.\n");
                        break;
                    case 7: 
                        currentPot = extractValue(msg, "POT_SIZE:");
                        printf("\n--- Betting Round Ended. Pot is now: %d ---\n", currentPot);
                        break;
                    case 8: {
                        const char *reason = strchr(msg, ':');
                        if (reason) reason++;
                        printf("\n*** HAND OVER*** %s\n", reason);
                        break;
                    }
                }
                handled = 1;
                break;
            }
        }

        if (!handled) {
            printf("Unhandled Server Msg: %s\n", msg);
        }
    }
}

#pragma endregion Game

#pragma region _Q

const char *mainMenu_Q() {
    const char *options[] = {"Create Game", "Join Game", "Settings", "Exit"};
    int count = sizeof(options) / sizeof(options[0]);
    return uul_cli_MultipleChoice(hConsole, options, count, "Enter IP or search LAN games");
}

char *enterName_Q() {
    char *name = malloc(PLAYER_NAME_LEN * sizeof(char));
    if (!name) return NULL;
    while (strcmp(name, "") == 0 || strlen(name) > 32) {
        printf("Enter name: ");
        fgets(name, sizeof(name), stdin);
    }
    return name;
}

const char *joinOrSearch_Q() {
    const char *options[3] = {"Enter IP", "Search LAN", "Back"};
    int count = sizeof(options) / sizeof(options[0]);
    const char *input = uul_cli_MultipleChoice(hConsole, options, count, "Enter IP or search LAN games");
    return input;
}

const char *noGamesFound_Q() {
    const char *options[1] = {"Back"};
    int count = sizeof(options) / sizeof(options[0]);
    const char *input = uul_cli_MultipleChoice(hConsole, options, count, "No games found");
    return input;
}

const char *settingsMenu_Q() {
    const char *options[3] = {"Client", "Server", "Back"};
    int count = sizeof(options) / sizeof(options[0]);
    const char *input = uul_cli_MultipleChoice(hConsole, options, count, "Edit settings");
    return input;
}

const char *settingsClientMenu_Q() {
    const char *options[2] = {"Change name", "Back"};
    int count = sizeof(options) / sizeof(options[0]);
    const char *input = uul_cli_MultipleChoice(hConsole, options, count, "Edit client settings");
    return input;
}

const char *settingsServerMenu_Q() {
    const char *options[4] = {"Small blind", "Big blind", "Players", "Back"};
    int count = sizeof(options) / sizeof(options[0]);
    const char *input = uul_cli_MultipleChoice(hConsole, options, count, "Edit client settings");
    return input;
}

#pragma endregion _Q

#pragma region IP Input

int isWordChar(char c) {
    return c >= '0' && c <= '9';
}

int isValidIPv4(const char *ip) {
    struct in_addr addr;
    return InetPtonA(AF_INET, ip, &addr) == 1;
}

int countDots(const char *s) {
    int count = 0;
    while (*s) {
        if (*s == '.') count++;
        s++;
    }
    return count;
}

int segmentLength(const char *s, int cursorPos) {
    int len = 0;
    int pos = cursorPos - 1;

    while (pos >= 0 && s[pos] != '.') {
        len++;
        pos--;
    }

    return len;
}

int canInsertChar(const char *ip, int cursorPos, char ch) {
    int dots = countDots(ip);

    if (ch == '.') {
        if (dots >= 3) return 0;
        if (cursorPos == 0) return 0;
        if (cursorPos > 0 && ip[cursorPos - 1] == '.') return 0;
        return 1;
    }

    if (ch >= '0' && ch <= '9') {
        int segLen = segmentLength(ip, cursorPos);
        if (segLen <= 3) return 0;
        return 1;
    }

    return 0;
}

char *getIPInput() {
    char *ip = malloc(16 * sizeof(char));
    if (!ip) return NULL;
    int validIp = 0;

    printf("Press ESC to cancel.\n");

    do {
        printf("Enter IP: ");

        COORD startPos = uul_cli_getCursorPosition(hConsole);
        int inputStartX = startPos.X;
        int cursorPos = 0;

        while (1) {
            if (_kbhit()) {
                int ch = _getch();

                if (ch == 27) return "";

                if (ch == 13) {
                    printf("\n");
                    break;
                }

                if (ch == 8 && cursorPos > 0) {
                    string_erase(ip, cursorPos - 1, 1);
                    cursorPos--;

                    printf("\r");
                    uul_cli_goToXY(hConsole, inputStartX, startPos.Y);
                    printf(ip + ' ');
                    uul_cli_goToXY(hConsole, inputStartX + cursorPos, startPos.Y);
                    continue;
                }

                if (ch == 127 || (GetAsyncKeyState(VK_CONTROL) & 0x8000) && ip[0]) {
                    int deleteStart = cursorPos;
                    int deleteEnd = cursorPos;
                    while (deleteStart > 0 && isWordChar(ip[deleteStart - 1])) deleteStart--;
                    while (deleteStart > 0 && !isWordChar(ip[deleteStart - 1])) deleteStart--;
                    if (deleteStart < deleteEnd) {
                        string_erase(ip, deleteStart, deleteEnd - deleteStart);
                        cursorPos = deleteStart;
                    }

                    printf("\r");
                    uul_cli_goToXY(hConsole, inputStartX, startPos.Y);
                    printf("%s    ", ip);
                    uul_cli_goToXY(hConsole, inputStartX + cursorPos, startPos.Y);

                    continue;
                }

                if (ch == 0 || ch == 224) {
                    int ch2 = _getch();
                    switch (ch2) {
                        // Up arrow
                        case 72:
                            cursorPos = 0;
                            uul_cli_goToXY(hConsole, inputStartX, startPos.Y);
                            break;
                        // Down arrow
                        case 80:
                            cursorPos = strlen(ip);
                            uul_cli_goToXY(hConsole, inputStartX + cursorPos, startPos.Y);
                            break;
                        // Left arrow
                        case 75:
                            if (cursorPos > 0) {
                                cursorPos--;
                                uul_cli_goToXY(hConsole, inputStartX + cursorPos, startPos.Y);
                            }
                            break;
                        // Right arrow
                        case 77:
                            if (cursorPos < strlen(ip)) {
                                cursorPos++;
                                uul_cli_goToXY(hConsole, inputStartX + cursorPos, startPos.Y);
                            }
                            break;
                        // Delete key
                        case 83:
                            string_erase(ip, cursorPos, 1);
                            printf("\r");
                            uul_cli_goToXY(hConsole, inputStartX, startPos.Y);
                            printf(ip + ' ');
                            uul_cli_goToXY(hConsole, inputStartX + cursorPos, startPos.Y);
                            break;
                    }
                    continue;
                }

                if ((ch >= '0' && ch <= '9') || ch == '.') {
                    if (!canInsertChar(ip, cursorPos, (char)ch)) {
                        continue;
                    }

                    string_insert(ip, ip[0] + cursorPos, (char[2]){(char)ch, '\0'}); // Yes that's a string
                    cursorPos++;
                    
                    printf("\r");
                    uul_cli_goToXY(hConsole, inputStartX, startPos.Y);
                    printf(ip);
                    uul_cli_goToXY(hConsole, inputStartX + cursorPos, startPos.Y);
                }
            }

            Sleep(1);
        }

        if (isValidIPv4(ip)) {
            validIp = 1;
        } else {
            printf("Invalid IP\n");
        }
    } while (!validIp);

    return ip;
}

#pragma endregion IP Input

#pragma region Menus

void selectGameMenu() {
    system(CLEAR_SCREEN_CMD);

    char **lanGames = uul_server_SearchForLANGames(&clientSocket);
    if (!lanGames[0]) {
        const char *options[] = {"Back"};
        const char *choice = uul_cli_MultipleChoice(hConsole, options, 1, "No games found.");
        if (strcmp(choice, "Back") == 0) {
            joinMenu();
        }
        return;
    }

    size_t lan_games_count = sizeof(lanGames) / sizeof(lanGames[0]);
    char **lanGameOptions = malloc((lan_games_count + 1) * sizeof(char *));

    for (size_t i = 0; i < lan_games_count; ++i) {
        lanGameOptions[i] = strcat(". ", lanGames[i]);
    }
    lanGameOptions[lan_games_count] = "Back";

    const char *selectedOption = uul_cli_MultipleChoice(hConsole, (const char **)lanGameOptions, lan_games_count + 1, "Pick a game");
    free(lanGameOptions);

    if (strcmp(selectedOption, "Back") == 0) {
        joinMenu();
        return;
    }
}

void joinMenu() {
    system(CLEAR_SCREEN_CMD);

    char name[33];
    strncpy(name, clientSettings.name, sizeof(name) - 1);
    name[sizeof(name) - 1] = '\0';

    if (name[0] == '\0') {
        printf("Seems like you're new.\n");
        strncpy(name, enterName_Q(), sizeof(name) - 1);
        strncpy(clientSettings.name, name, sizeof(clientSettings.name) - 1);
        clientSettings.name[sizeof(clientSettings.name) - 1] = '\0';
        saveSettings("config.json");
        system(CLEAR_SCREEN_CMD);
    }

    printf("Welcome back %s!\n", name);
    const char *joinOrSearch = joinOrSearch_Q();

    if (strcmp(joinOrSearch, "Enter IP") == 0) {
        enterIpMenu();
    } else if (strcmp(joinOrSearch, "Search LAN") == 0) {
        selectGameMenu();
    } else if (strcmp(joinOrSearch, "Back") == 0) {
        main_menu();
    }
}

void settingsClientMenu() {
    system(CLEAR_SCREEN_CMD);

    const char *input = settingsClientMenu_Q();

    if (strcmp(input, "Back") == 0) {
        settingsMenu();
        return;
    } else if (strcmp(input, "Change name") == 0) {
        system(CLEAR_SCREEN_CMD);
        printf("Current name: %s\n", clientSettings.name);
        char *name = enterName_Q();
        strncpy(clientSettings.name, name, sizeof(clientSettings.name) - 1);
        settingsClientMenu();
    }

    saveSettings("config.json");
}

void settingsServerMenu() {
    system(CLEAR_SCREEN_CMD);

    const char *input = settingsServerMenu_Q();
    char buffer[128];

    if (strcmp(input, "Back") == 0) {
        settingsMenu();
        return;
    } else if (strcmp(input, "Big blind") == 0) {
        printf("Current big blind: %d\n", serverSettings.bigBlind);
        printf("Edit big blind: ");
        fgets(buffer, sizeof(buffer), stdin);
        serverSettings.bigBlind = strtol(buffer, NULL, 10);
    } else if (strcmp(input, "Small blind") == 0) {
        printf("Current small blind: %d\n", serverSettings.smallBlind);
        printf("Edit small blind: ");
        fgets(buffer, sizeof(buffer), stdin);
        serverSettings.smallBlind = strtol(buffer, NULL, 10);
    } else if (strcmp(input, "Players") == 0) {
        int playerCount;
        while (playerCount < 3) {
            printf("Current player count: %d\n", serverSettings.players);
            printf("Edit player count: ");
            fgets(buffer, sizeof(buffer), stdin);
            playerCount = strtol(buffer, NULL, 10);
            if (playerCount < 3) {
                printf("Minimum of 3 players\n");
            } else if (playerCount > 22) {
                printf("Maximum of 22 players\n");
            }
        }
        serverSettings.players = playerCount;
    } else if (strcmp(input, "Starting chips") == 0) {
        printf("Current starting chip count: %d\n", serverSettings.startingChips);
        printf("Edit starting chip amount: ");
        fgets(buffer, sizeof(buffer), stdin);
        serverSettings.startingChips = strtol(buffer, NULL, 10);
    }

    saveSettings("config.json");
    settingsServerMenu();
}

void settingsMenu() {
    system(CLEAR_SCREEN_CMD);

    char name[33];
    strncpy(name, clientSettings.name, sizeof(name) - 1);
    const char *input = settingsMenu_Q();

    if (strcmp(input, "Back") == 0) {
        main_menu();
    } else if (strcmp(input, "Client") == 0) {
        settingsClientMenu();
    } else if (strcmp(input, "Server") == 0) {
        settingsServerMenu();
    }
}

void enterIpMenu() {
    system(CLEAR_SCREEN_CMD);
    
    char *ip = getIPInput();
    if (ip[0] == '\0') {
        joinMenu();
    }

    printf("Attempting to join: %s\n", ip);
    clientJoinGame(ip);
}

void main_menu() {
    system(CLEAR_SCREEN_CMD);
    const char *input = mainMenu_Q();

    if (strcmp(input, "Join Game") == 0) {
        joinMenu();
    } else if (strcmp(input, "Create Game") == 0) {
        createGame();
    } else if (strcmp(input, "Settings") == 0) {
        settingsMenu();
    } else if (strcmp(input, "Exit")) {
        return;
    }
}

#pragma endregion Menus

#pragma region Main

int main(int argc, char *argv[]) {
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    uul_cli_EnableUTF8();

    char error[128];
    if (!loadSettings("config.json", error, sizeof(error))) {
        show_windows_error_dialog(L"Loading settings failed.", L"Failed to load settings from config.json: ");
        return 1;
    }

    if (argc > 1 && strcmp(argv[1], "Server") == 0) {
        createGame();
    }
    
    if (!uul_server_Initialize(&clientSocket)) {
        printf("Client socket init failed\n");
        return 1;
    }

    if (argc > 2 && strcmp(argv[1], "join") == 0 && isValidIPv4(argv[2])) {
        clientJoinGame(argv[2]);
    } else {
        main_menu();
    }

    uul_server_Cleanup((SOCKET []){clientSocket}, 1);

    return 0;
}

#pragma endregion Main