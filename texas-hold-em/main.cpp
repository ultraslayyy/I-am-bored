#define NOMINMAX

#include <iostream>
#include <vector>
#include <cstdlib>
#include <thread>
#include <memory>
#include <regex>
#include <random>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <array>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <conio.h>
#include <windows.h>

#include "ultras_utils.hpp"

#define JSON_NO_SAX
#define JSON_NO_DLL
#define JSON_NO_SERIALIZATION
#include "include/json.hpp"

const std::string CLEAR_SCREEN_CMD = "cls";

uul::CLI cliHandler;
uul::Server serverHandler;

void mainMenu();
void joinMenu();
void settingsMenu();
void selectGameMenu();
void enterIPMenu();
void clientGame(SOCKET cSocket);

class Player {
public:
    explicit Player(SOCKET socket_handle) : socket_handle_(socket_handle) {}

    ~Player() {
        if (socket_handle_ != INVALID_SOCKET) {
            std::cout << "Closing socket for player." << std::endl;
            closesocket(socket_handle_);
            socket_handle_ = INVALID_SOCKET;
        }
    }

    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;
    Player(Player&) = default;
    Player& operator=(Player&&) = default;

    std::string name = "Guest";

    // In-game tracking
    int chips;
    int pIndex = -1;
    std::array<std::string, 2> hand;
    bool folded = false;
    bool allIn = false;
    int roundBet = 0;

    SOCKET getSocket() const {
        return socket_handle_;
    }
private:
    SOCKET socket_handle_;
};

void ShowWindowsErrorDialog(const wchar_t* title, const wchar_t* message) {
    int result = MessageBoxW(
        NULL,
        message,
        title,
        MB_ICONERROR | MB_OK
    );
}

struct ClientSettings {
    std::string name = "Guest";
} clientSettings;

struct ServerSettings {
    int smallBlind = 10;
    int bigBlind = 20;
    int players = 4;
    int startingChips = 1000;
} serverSettings;

#pragma region JSON

void to_json(nlohmann::json& j, const ClientSettings& c) {
    j = nlohmann::json{
        {"name", c.name}  
    };
}

void from_json(const nlohmann::json& j, ClientSettings& c) {
    j.at("name").get_to(c.name);
}

void to_json(nlohmann::json& j, const ServerSettings& s) {
    j = nlohmann::json{
        {"smallBlind", s.smallBlind},
        {"bigBlind", s.bigBlind},
        {"players", s.players},
        {"startingChips", s.startingChips}
    };
}

void from_json(const nlohmann::json& j, ServerSettings& s) {
    j.at("smallBlind").get_to(s.smallBlind);
    j.at("bigBlind").get_to(s.bigBlind);
    j.at("players").get_to(s.players);
    j.at("startingChips").get_to(s.startingChips);
}

bool saveSettings(const ServerSettings& s = serverSettings, const ClientSettings& c = clientSettings, std::string filename = "config.json") {
    nlohmann::json j;
    j["server"] = s;
    j["client"] = c;

    uul::File file(filename);
    file << j.dump(2);

    return file.wasSuccessful();
}

bool loadSettings(const std::string& filename, std::string& error) {
   uul::File file(filename);
    
    try {
        nlohmann::json j;
        file >> j;
        if (!file.wasSuccessful()) throw;

        serverSettings = j.at("server").get<ServerSettings>();
        clientSettings = j.at("client").get<ClientSettings>();
        return true;
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "Error: JSON file is corrupt/invalid (" << filename << "). " << e.what() << std::endl;
        return false;
    } catch (const nlohmann::json::out_of_range& e) {
        std::cerr << "Error: Settings file is missing required key ('server' or 'client'). " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred during loading: " << e.what() << std::endl;
        return false;
    }
}

#pragma endregion JSON

#pragma region Game

std::vector<std::string> deck;
std::default_random_engine rng(std::chrono::system_clock::now().time_since_epoch().count());

void genDeck() {
    deck.clear();
    std::array<std::string, 4> suits = {"\u2665", "\u2660", "\u2666", "\u2663"};
    std::array<std::string, 13> ranks = {"A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"};

    deck.reserve(52);
    for (const auto& suit : suits) {
        for (const auto& rank : ranks) {
            deck.push_back(rank + suit);
        }
    }
    std::shuffle(deck.begin(), deck.end(), rng);
}

std::string genCard() {
    if (deck.empty()) {
        throw std::runtime_error("Deck is empty");
    }
    std::string card = deck.back();
    deck.pop_back();
    return card;
}

void broadcastRequest(std::vector<std::reference_wrapper<Player>>& players, std::string msg) {
    for (size_t i = 0; i < players.size(); ++i) {
        serverHandler.SendRequest(players[i].get().getSocket(), msg);
    }
}

void broadcastRequest(std::vector<std::unique_ptr<Player>>& players, std::string msg) {
    for (size_t i = 0; i < players.size(); ++i) {
        serverHandler.SendRequest(players[i]->getSocket(), msg);
    }
}

void bettingRound(std::vector<std::unique_ptr<Player>>& players, std::unique_ptr<int>& pot, const std::string& phase) {
    int currentBet = (phase == "PRE_FLOP") ? serverSettings.bigBlind : 0;
    size_t lastRaiserIndex = (phase == "PRE_FLOP") ? 1 : players.size();

    size_t startingPlayerIndex;
    
    if (phase == "PRE_FLOP") {
        startingPlayerIndex = 3 % players.size();
        lastRaiserIndex = 2;
    } else {
        startingPlayerIndex = 1;
        lastRaiserIndex = players.size();
    }

    for (auto& p : players) {
        if (!p->folded) {
            if (phase == "PRE_FLOP") {
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
        Player* currentPlayer = nullptr;
        for (const auto& p : players) {
            if (p->pIndex == (int)currentPlayerIndex) {
                currentPlayer = p.get();
                break;
            }
        }
        
        if (!currentPlayer || currentPlayer->folded || (currentPlayer->allIn && currentPlayer->roundBet >= currentBet)) {
            currentPlayerIndex = (currentPlayerIndex + 1) % players.size();
            continue; 
        }

        int maxRoundBet = 0;
        int activePlayersCount = 0;
        bool allActivePlayersSatisfied = true;
        
        for (auto& p : players) {
            if (!p->folded) {
                activePlayersCount++;
                if (p->roundBet > maxRoundBet) {
                    maxRoundBet = p->roundBet;
                }
                
                if (!p->allIn && p->roundBet < currentBet) {
                    allActivePlayersSatisfied = false;
                }
            }
        }
        currentBet = maxRoundBet;
        
        if (activePlayersCount <= 1) {
            broadcastRequest(players, "HAND_OVER:ONLY_ONE_PLAYER");
            return;
        }

        if (allActivePlayersSatisfied && currentPlayerIndex == lastRaiserIndex) {
            break;
        }
        
        /*
        if (currentPlayer->roundBet >= currentBet && currentPlayerIndex != lastRaiserIndex) {
            currentPlayerIndex = (currentPlayerIndex + 1) & players.size();
            continue;
        } */
        currentPlayerIndex = (currentPlayerIndex + 1) & players.size();

        std::cout << "Hereo" << std::endl;

        int amountToCall = currentBet - currentPlayer->roundBet;

        std::string requestMsg = "YOUR_TURN:";
        requestMsg += "CALL:" + std::to_string(amountToCall);
        requestMsg += "|MIN_RAISE:" + std::to_string(currentBet + amountToCall); 
        requestMsg += "|CHIPS:" + std::to_string(currentPlayer->chips);

        serverHandler.SendRequest(currentPlayer->getSocket(), requestMsg);

        std::string actionMsg = serverHandler.ReceiveRequest(currentPlayer->getSocket());
        std::string action = actionMsg.substr(0, actionMsg.find(' '));
        int amount = 0;
        try {
            if (actionMsg.find(' ') != std::string::npos) {
                amount = std::stoi(actionMsg.substr(actionMsg.find(' ') + 1));
            }
        } catch (...) {  }

        int amountCommitted = 0;
        
        if (action == "FOLD") {
            currentPlayer->folded = true;
            broadcastRequest(players, "ACTION:" + currentPlayer->name + ":FOLD");
        } else if (action == "CALL" || (action == "RAISE" && amount < currentBet)) {
            amountCommitted = amountToCall;
            broadcastRequest(players, "ACTION:" + currentPlayer->name + ":CALL " + std::to_string(amountToCall));
        } else if (action == "RAISE") {
            amountCommitted = amount - currentPlayer->roundBet;
            
            if (amount > currentBet && amountCommitted <= currentPlayer->chips) {
                if (amount - currentBet < (currentBet - (currentBet - amountToCall))) {
                    // TODO: fix
                }

                currentBet = amount;
                lastRaiserIndex = currentPlayerIndex;
                broadcastRequest(players, "ACTION:" + currentPlayer->name + ":RAISE to " + std::to_string(amount));
            } else {
                amountCommitted = amountToCall; 
                broadcastRequest(players, "ACTION:" + currentPlayer->name + ":CALL (Invalid RAISE)");
            }
        }
        
        if (amountCommitted > 0) {
            int actualCommit = std::min(amountCommitted, currentPlayer->chips);
            
            currentPlayer->chips -= actualCommit;
            currentPlayer->roundBet += actualCommit;
            *pot += actualCommit;

            if (currentPlayer->chips == 0) {
                currentPlayer->allIn = true;
                broadcastRequest(players, "ACTION:" + currentPlayer->name + ":ALL-IN!");
            }
        }

        currentPlayerIndex = (currentPlayerIndex + 1) % players.size();
    } while (true); 

    broadcastRequest(players, "END_BETTING_ROUND:POT_SIZE:" + std::to_string(*pot));
}

void runBlinds(std::vector<std::unique_ptr<Player>>& players, std::unique_ptr<int>& pot) {
    for (auto& p : players) {
        if (p->pIndex == 1) {
            p->chips -= serverSettings.smallBlind;
            broadcastRequest(players, "small_blind:" + p->name);
        } else if (p->pIndex == 2) {
            p->chips -= serverSettings.bigBlind;
            broadcastRequest(players, "big_blind:" + p->name);
        }
    }

    *pot += serverSettings.smallBlind;
    *pot += serverSettings.bigBlind;
}

void randomisePlayerOrder(std::vector<std::unique_ptr<Player>>& players) {
    std::vector<int> indices(players.size());
    for (size_t i = 0; i < players.size(); ++i) indices[i] = static_cast<int>(i);

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(indices.begin(), indices.end(), g);

    for (size_t i = 0; i < players.size(); ++i) players[i]->pIndex = indices[i];
}

void serverGame(std::vector<std::unique_ptr<Player>>& players) {
    genDeck();

    std::unique_ptr<int> pot = std::make_unique<int>(0);
    std::array<std::string, 5> comCards;

    randomisePlayerOrder(players);
    runBlinds(players, pot);

    for (const auto& p : players) {
        p->hand = {genCard(), genCard()};
        std::string handStr = p->hand[0] + " " + p->hand[1];
        serverHandler.SendRequest(p->getSocket(), handStr);
    }
    broadcastRequest(players, "HOLE_CARDS_DEALT");
    broadcastRequest(players, "PHASE:PRE_FLOP|POT:" + std::to_string(*pot));
    bettingRound(players, pot, "PRE_FLOP");

    comCards[0] = genCard();
    comCards[1] = genCard();
    comCards[2] = genCard();
    std::string flopStr = "COMMUNITY_CARDS:" + comCards[0] + " " + comCards[1] + " " + comCards[2];

    broadcastRequest(players, "PHASE:FLOP|POT:" + std::to_string(*pot));
    broadcastRequest(players, flopStr);
    bettingRound(players, pot, "POST_FLOP");

    comCards[3] = genCard();
    std::string turnStr = flopStr + " " + comCards[3];

    broadcastRequest(players, "PHASE:TURN|POT:" + std::to_string(*pot));
    broadcastRequest(players, turnStr);
    bettingRound(players, pot, "POST_FLOP");

    comCards[4] = genCard();
    std::string riverStr = turnStr + " " + comCards[4];

    broadcastRequest(players, "PHASE:RIVER|POT:" + std::to_string(*pot));
    broadcastRequest(players, riverStr);
    bettingRound(players, pot, "POST_FLOP");

    broadcastRequest(players, "PHASE:SHOWDOWN|POT:" + std::to_string(*pot));
}

void RunDiscoveryListener() {
    std::cout << "Starting Discovery Listener thread..." << std::endl;
    serverHandler.RunDiscoveryLoop();
    std::cerr << "Discovery Listener thread terminated!" << std::endl;
}

void createGame() {
    std::system(CLEAR_SCREEN_CMD.c_str());
    std::cout << "Attempting to create a new game server..." << std::endl;

    auto shared_server = std::make_shared<uul::Server>();
    std::thread discovery_thread([shared_server](){
        if (!shared_server->StartDiscoveryListener(shared_server->DEFAULT_PORT)) {
            std::cerr << "FATAL: Failed to start UDP Discovery Listener. Check binding/port." << std::endl;
            mainMenu();
            return; 
        }

        shared_server->RunDiscoveryLoop();
    });
    discovery_thread.detach();

    if (!serverHandler.CreateServer(serverHandler.DEFAULT_PORT)) {
        std::cerr << "FATAL: Failed to start TCP Game Server. Check binding/port." << std::endl;
        serverHandler.Cleanup();
        return; 
    }

    std::cout << "\nGame Server is now LIVE (TCP & UDP on port " << serverHandler.DEFAULT_PORT << ")." << std::endl;
    std::cout << "Clients can now find and join your game." << std::endl;
    
    std::vector<std::unique_ptr<Player>> connected_players;
    const int MAX_CLIENTS = serverSettings.players;
    connected_players.reserve(MAX_CLIENTS);

    while (connected_players.size() < MAX_CLIENTS) {
        std::cout << "\nWaiting for " << (MAX_CLIENTS - connected_players.size()) << " more player(s)..." << std::endl;
        
        SOCKET new_client_socket = serverHandler.AcceptClient(); 

        if (new_client_socket != INVALID_SOCKET) {
            std::string playerName = serverHandler.ReceiveRequest(new_client_socket);

            connected_players.push_back(std::make_unique<Player>(new_client_socket));
            connected_players.back()->name = playerName;
            connected_players.back()->chips = serverSettings.startingChips;
            
            std::cout << "Client connected! Total players: " << connected_players.size() << "/" << MAX_CLIENTS << std::endl;

            std::stringstream msg;
            msg << "player_connected:" << connected_players.back()->name;
            
            std::vector<std::reference_wrapper<Player>> existingPlayers;
            existingPlayers.reserve(connected_players.size() - 1);
            for (size_t i = 0; i < connected_players.size() - 1; ++i) {
                existingPlayers.push_back(*connected_players[i].get());
            }

            broadcastRequest(existingPlayers, msg.str());
        } else {
            std::cerr << "Error accepting client. Exiting server loop." << std::endl;
            break; 
        }
    }
    
    std::cout << "\n--- Game session starting with " << connected_players.size() << " players! ---" << std::endl;

    broadcastRequest(connected_players, "game_start");

    serverGame(connected_players);
    
    cliHandler.MultipleChoice({"Stop Server"}, "Press to end game and stop server.");
    
    serverHandler.Cleanup();
}

void clientJoinGame(std::string ip) {
    if (serverHandler.ConnectClient(ip, serverHandler.DEFAULT_PORT)) {
        std::string name = clientSettings.name;

        SOCKET clientSocket = serverHandler.GetClientSocket();
        serverHandler.SendRequest(clientSocket, name);

        // Make cool TUI with player list, leave option, etc.

        std::cout << "Starting game session with server!" << std::endl;

        const std::array<std::string, 2> serverRequestPrefixes = {
            "player_connected:",
            "game_start"
        };

        bool waiting = true;
        while (waiting) {
            std::string msg = serverHandler.ReceiveRequest(clientSocket);

            for (size_t i = 0; i < serverRequestPrefixes.size(); ++i) {
                if (msg.compare(0, serverRequestPrefixes[i].size(), serverRequestPrefixes[i]) == 0) {
                    switch (i) {
                        case 0: {
                            size_t cPos = msg.find(':');
                            if (cPos != std::string::npos) {
                                std::string name = msg.substr(cPos + 1);
                                std::cout << "Player connected: " << name << std::endl;
                            }
                            break;
                        }
                        case 1:
                            clientGame(clientSocket);
                            break;
                    }
                    break;
                }
            }
        }

        clientGame(clientSocket);
    } else {
        std::cerr << "Failed to connect to " << ip << ". Returning to join menu." << std::endl;
        joinMenu();
    }
}

int extractValue(const std::string& msg, const std::string& key) {
    size_t keyPos = msg.find(key);
    if (keyPos == std::string::npos) return 0;

    size_t startPos = keyPos + key.length();
    size_t endPos = msg.find('|', startPos);
    if (endPos == std::string::npos) endPos = msg.length();

    try {
        std::string valueStr = msg.substr(startPos, endPos - startPos);
        return std::stoi(valueStr);
    } catch (...) {
        return 0;
    }
}

void clientGame(SOCKET cSocket) {
    std::system(CLEAR_SCREEN_CMD.c_str());
    std::cout << "Game started! Waiting for action..." << std::endl;

    const std::array<std::string, 9> serverRequestPrefixes = {
        "small_blind:",
        "big_blind:",
        "COMMUNITY_CARDS:",
        "PHASE:",
        "YOUR_TURN:",
        "ACTION:",
        "HOLE_CARDS_DEALT",
        "END_BETTING_ROUND:",
        "HAND_OVER"
    };

    bool gameRunning = true;
    std::string playerHand = "";
    std::string communityCards = "";
    int currentPot = 0;

    while (gameRunning) {
        std::string msg = serverHandler.ReceiveRequest(cSocket);

        if (msg.find_first_of("\u2665\u2660\u2666\u2663") != std::string::npos && msg.size() < 10 && msg.find(':') == std::string::npos) {
            playerHand = msg;
            std::cout << "\n** Your Hand: " << playerHand << " **" << std::endl;
            continue;
        }

        bool handled = false;
        for (size_t i = 0; i < serverRequestPrefixes.size(); ++i) {
            if (msg.compare(0, serverRequestPrefixes[i].size(), serverRequestPrefixes[i]) == 0) {
                switch (i) {
                    case 0:
                    case 1:
                        std::cout << "BLIND: " << msg.substr(msg.find(':') + 1) << "posts a blind." << std::endl;
                        break;
                    case 2:
                        communityCards = msg.substr(msg.find(':') + 1);
                        std::cout << "\n*** COMMUNITY CARDS: " << communityCards << " ***" << std::endl;
                        break;
                    case 3: {
                        std::string phaseName = msg.substr(msg.find(':') + 1, msg.find('|') - msg.find(':') - 1);
                        currentPot = extractValue(msg, "POT:");
                        std::cout << "\n--- Starting " << phaseName << " Phase (Pot: " << currentPot << ") ---" << std::endl;
                        break;
                    }
                    case 4: {
                        int callAmount = extractValue(msg, "CALL:");
                        int minRaise = extractValue(msg, "MIN_RAISE:");
                        int chips = extractValue(msg, "CHIPS:");
                        
                        std::cout << "\n--- IT IS YOUR TURN! ---" << std::endl;
                        std::cout << "Your Hand: " << playerHand << " | Community: " << communityCards << " | Pot: " << currentPot << std::endl;
                        std::cout << "Chips: " << chips << std::endl;
                        
                        std::string action;
                        do {
                            std::cout << "Enter action (FOLD, CALL (" << callAmount << "), RAISE [>" << minRaise << "]): ";
                            std::getline(std::cin, action);
                            
                            if (action.empty()) continue;

                        } while (action.empty());

                        serverHandler.SendRequest(cSocket, action); 
                        break;
                    }
                    case 5: {
                        std::cout << "Player Action: " << msg.substr(msg.find(':') + 1) << std::endl;
                        break;
                    }
                    case 6: {
                        std::cout << "Hole cards dealt to all players." << std::endl;
                        break;
                    }
                    case 7: {
                         currentPot = extractValue(msg, "POT_SIZE:");
                         std::cout << "\n--- Betting Round Ended. Pot is now: " << currentPot << " ---" << std::endl;
                         break;
                    }
                    case 8: {
                        std::cout << "\n*** HAND OVER *** " << msg.substr(msg.find(':') + 1) << std::endl;
                        break;
                    }
                }
                handled = true;
                break;
            }
        }

        if (!handled) {
            std::cout << "Unhandled Server Msg: " << msg << std::endl;
        }
    }
}

#pragma endregion Game

#pragma region _Q

std::string mainMenu_Q() {
    return cliHandler.MultipleChoice({"Create Game", "Join Game", "Settings", "Exit"}, "Join or create game.");
}

std::string enterName_Q() {
    std::string name;
    name.reserve(32);
    while (name == "" || name.size() > 32) {
        std::cout << "Enter name: ";
        std::getline(std::cin, name);
    }
    return name;
}

std::string joinOrSearch_Q() {
    return cliHandler.MultipleChoice({"Enter IP", "Search LAN", "Back"}, "Enter IP or search LAN games");
}

std::string noGamesFound_Q() {
    return cliHandler.MultipleChoice({"Back"}, "No games found");
}

std::string settingsMenu_Q() {
    return cliHandler.MultipleChoice({"Client", "Server", "Back"}, "Edit settings");
}

std::string settingsClientMenu_Q() {
    return cliHandler.MultipleChoice({"Change name", "Back"}, "Edit client settings");
}

std::string settingsServerMenu_Q() {
    return cliHandler.MultipleChoice({"Small blind", "Big blind", "Players", "Back"}, "Edit server settings");
}

#pragma endregion _Q

#pragma region IP Input

bool isWordChar(char c) {
    return c >= '0' && c <= '9';
}

bool isValidIPv4(const std::string& ip) {
    in_addr addr;
    return InetPtonA(AF_INET, ip.c_str(), &addr) == 1;
}

int countDots(const std::string& s) {
    return std::count(s.begin(), s.end(), '.');
}

int segmentLength(const std::string& s, int cursorPos) {
    int len = 0;
    int pos = cursorPos - 1;

    while (pos >= 0 && s[pos] != '.') {
        len++;
        pos--;
    }

    return len;
}

bool canInsertChar(const std::string& ip, int cursorPos, char ch) {
    int dots = countDots(ip);

    if (ch == '.') {
        if (dots >= 3) return false;
        if (cursorPos == 0) return false;
        if (cursorPos > 0 && ip[cursorPos - 1] == '.') return false;
        return true;
    }

    if (ch >= '0' && ch <= '9') {
        int segLen = segmentLength(ip, cursorPos);
        if (segLen >= 3) return false;
        return true;
    }

    return false;
}

std::string getIPInput() {
    std::string ip;
    ip.reserve(15);
    bool validIp = false;

    std::cout << "Press ESC to cancel.\n";

    do {
        ip.clear();
        std::cout << "Enter IP: ";

        COORD startPos = cliHandler.getCursorPosition();
        int inputStartX = startPos.X;
        int cursorPos = 0;

        while (true) {
            if (_kbhit()) {
                int ch = _getch();

                if (ch == 27) return "";

                if (ch == 13) {
                    std::cout << "\n";
                    break;
                }

                if (ch == 8 && cursorPos > 0) {
                    ip.erase(cursorPos - 1, 1);
                    cursorPos--;

                    std::cout << "\r";
                    cliHandler.goToXY(inputStartX, startPos.Y);
                    std::cout << ip << ' ';
                    cliHandler.goToXY(inputStartX + cursorPos, startPos.Y);
                    continue;
                }

                if (ch == 127 || (GetAsyncKeyState(VK_CONTROL) & 0x8000) && !ip.empty()) {
                    int deleteStart = cursorPos;
                    int deleteEnd = cursorPos;
                    while (deleteStart > 0 && isWordChar(ip[deleteStart - 1])) deleteStart--;
                    while (deleteStart > 0 && !isWordChar(ip[deleteStart - 1])) deleteStart--;
                    if (deleteStart < deleteEnd) {
                        ip.erase(deleteStart, deleteEnd - deleteStart);
                        cursorPos = deleteStart;
                    }

                    std::cout << "\r";
                    cliHandler.goToXY(inputStartX, startPos.Y);
                    std::cout << ip << "    ";
                    cliHandler.goToXY(inputStartX + cursorPos, startPos.Y);

                    continue;
                }

                if (ch == 0 || ch == 224) {
                    int ch2 = _getch();
                    switch (ch2) {
                        // Up arrow
                        case 72:
                            cursorPos = 0;
                            cliHandler.goToXY(inputStartX, startPos.Y);
                            break;
                        // Down arrow
                        case 80:
                            cursorPos = ip.size();
                            cliHandler.goToXY(inputStartX + ip.size(), startPos.Y);
                            break;
                        // Left arrow
                        case 75:
                            if (cursorPos > 0) {
                                cursorPos--;
                                cliHandler.goToXY(inputStartX + cursorPos, startPos.Y);
                            }
                            break;
                        // Right arrow
                        case 77:
                            if (cursorPos < ip.size()) {
                                cursorPos++;
                                cliHandler.goToXY(inputStartX + cursorPos, startPos.Y);
                            }
                            break;
                        // Delete key
                        case 83:
                            ip.erase(cursorPos, 1);
                            std::cout << "\r";
                            cliHandler.goToXY(inputStartX, startPos.Y);
                            std::cout << ip << ' ';
                            cliHandler.goToXY(inputStartX + cursorPos, startPos.Y);
                            break;
                    }
                    continue;
                }

                if ((ch >= '0' && ch <= '9') || ch == '.') {
                    if (!canInsertChar(ip, cursorPos, (char)ch)) {
                        continue;
                    }

                    ip.insert(ip.begin() + cursorPos, (char)ch);
                    cursorPos++;
                    
                    std::cout << "\r";
                    cliHandler.goToXY(inputStartX, startPos.Y);
                    std::cout << ip;
                    cliHandler.goToXY(inputStartX + cursorPos, startPos.Y);
                }
            }

            Sleep(1);
        }

        if (isValidIPv4(ip)) {
            validIp = true;
        } else {
            std::cout << "Invalid IP" << std::endl;
        }
    } while (!validIp);

    return ip;
}

#pragma endregion IP Input

#pragma region Menus

void selectGameMenu() {
    std::system(CLEAR_SCREEN_CMD.c_str());

    std::vector<std::string> lanGames = serverHandler.SearchForLANGames();

    if (lanGames.empty()) {
        std::string choice = cliHandler.MultipleChoice({"Back"}, "No games found.");
        if (choice == "Back") {
            joinMenu();
        }
        return;
    }
    
    std::vector<std::string> lanGameOptions;
    lanGameOptions.reserve(lanGames.size() + 1);

    for (size_t i = 0; i < lanGames.size(); ++i) {
        lanGameOptions.push_back(std::to_string(i + 1) + ". " + lanGames[i]);
    }
    lanGameOptions.push_back("Back");
    
    std::string selectedOption = cliHandler.MultipleChoice(lanGameOptions, "Pick a game");

    if (selectedOption == "Back") {
        joinMenu();
        return;
    }

    try {
        size_t index = std::stoul(selectedOption.substr(0, selectedOption.find('.'))) -1;
        if (index < lanGames.size()) {
            std::string selectedGameIP = lanGames[index];
            std::cout << "Attemtping to join game at: " << selectedGameIP << std::endl;

            clientJoinGame(selectedGameIP);
            while (true) {}
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing selection: " << selectedOption << ". Returning to join menu." << std::endl;
        joinMenu();
    }
}

void joinMenu() {
    std::system(CLEAR_SCREEN_CMD.c_str());

    std::string name = clientSettings.name;
    if (name == "") {
        std::cout << "Seems like you're new." << std::endl;
        name = enterName_Q();
        clientSettings.name = name;
        saveSettings();
        std::system(CLEAR_SCREEN_CMD.c_str());
    }

    std::cout << "Welcome back " << name << "!" << std::endl;
    std::string joinOrSearch = joinOrSearch_Q();

    if (joinOrSearch == "Enter IP") {
        enterIPMenu();
    } else if (joinOrSearch == "Search LAN") {
        selectGameMenu();
    } else if (joinOrSearch == "Back") {
        mainMenu();
    }
}

void settingsClientMenu() {
    std::system(CLEAR_SCREEN_CMD.c_str());

    std::string input = settingsClientMenu_Q();

    if (input == "Back") {
        settingsMenu();
        return;
    } else if (input == "Change name") {
        std::system(CLEAR_SCREEN_CMD.c_str());
        std::cout << "Current name: " << clientSettings.name << std::endl;
        // TODO: Give `esc` cancel button
        clientSettings.name = enterName_Q();
        settingsClientMenu();
    }

    saveSettings();
}

void settingsServerMenu() {
    std::system(CLEAR_SCREEN_CMD.c_str());

    std::string input = settingsServerMenu_Q();

    if (input == "Back") {
        settingsMenu();
        return;
    } else if (input == "Big blind") {
        std::cout << "Current big blind: " << serverSettings.bigBlind << std::endl;
        std::cout << "Edit big blind: ";
        std::cin >> serverSettings.bigBlind;
    } else if (input == "Small blind") {
        std::cout << "Current small blind: " << serverSettings.smallBlind << std::endl;
        std::cout << "Edit small blind: ";
        std::cin >> serverSettings.smallBlind;
    } else if (input == "Players") {
        int playerCount;
        while (playerCount < 3) {
            std::cout << "Current player count: " << serverSettings.players << std::endl;
            std::cout << "Edit player count: ";
            std::cin >> playerCount;
            if (playerCount < 3) {
                std::cout << "Minimum of 3 players" << std::endl;
            } else if (playerCount > 22) {
                std::cout << "Maximum of 22 players" << std::endl;
            }
        }
        serverSettings.players = playerCount;
    } else if (input == "Starting chips") {
        std::cout << "Current starting chip count: " << serverSettings.startingChips << std::endl;
        std::cout << "Edit starting chip amount: ";
        std::cin >> serverSettings.startingChips;
    }

    saveSettings();
    settingsServerMenu();
}

void settingsMenu() {
    std::system(CLEAR_SCREEN_CMD.c_str());

    std::string name = clientSettings.name;
    std::string input = settingsMenu_Q();

    if (input == "Back") {
        mainMenu();
    } else if (input == "Client") {
        settingsClientMenu();
    } else if (input == "Server") {
        settingsServerMenu();
    }
}

void enterIPMenu() {
    std::system(CLEAR_SCREEN_CMD.c_str());

    std::string ip = getIPInput();
    if (ip.empty()) {
        joinMenu();
    }

    std::cout << "Attemtping to join: " << ip << std::endl;
    clientJoinGame(ip);
}

void mainMenu() {
    std::system(CLEAR_SCREEN_CMD.c_str());
    std::string input = mainMenu_Q();
    
    if (input == "Join Game") {
        joinMenu();
    } else if (input == "Create Game") {
        createGame();
    } else if (input == "Settings") {
        settingsMenu();
    } else if (input == "Exit") {
        return;
    }
}

#pragma endregion Menus

#pragma region Main

int main(int argc, char* argv[]) {
    cliHandler.EnableUTF8();

    std::string loadSettingsError;
    if (!loadSettings("config.json", loadSettingsError)) {
        const wchar_t* title = L"Loading settings failed.";
        std::wstring wError(loadSettingsError.begin(), loadSettingsError.end());
        std::wstring msg = L"Failed to load settings from config.json: " + wError;
        const wchar_t* message = msg.c_str();

        ShowWindowsErrorDialog(title, message);
    }

    if (argc > 1 && std::string(argv[1]) == "server") {
        createGame();
    } else if (argc > 2 && std::string(argv[1]) == "join" && isValidIPv4(std::string(argv[2]))) {
        clientJoinGame(std::string(argv[2]));
    } else {
        mainMenu();
    }

    return 0;
}

#pragma endregion Main