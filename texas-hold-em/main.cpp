#include <iostream>
#include <vector>
#include <cstdlib>
#include <thread>
#include <memory>
#include <winsock2.h>

#include "ultras_utils.hpp"

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
    int chips = 1000;

    SOCKET getSocket() const {
        return socket_handle_;
    }
private:
    SOCKET socket_handle_;
};

const std::string CLEAR_SCREEN_CMD = "cls";

uul::CLI cliHandler;
uul::Server serverHandler;

void mainMenu();
void joinMenu();
void selectGameMenu();
void enterIPMenu();

void RunDiscoveryListener() {
    std::cout << "Starting Discovery Listener thread..." << std::endl;
    serverHandler.RunDiscoveryLoop();
    std::cerr << "Discovery Listener thread terminated!" << std::endl;
}

void createGame() {
    std::system(CLEAR_SCREEN_CMD.c_str());
    std::cout << "Attempting to create a new game server..." << std::endl;

    if (!serverHandler.StartDiscoveryListener(uul::DEFAULT_PORT)) {
        std::cerr << "FATAL: Failed to start UDP Discovery Listener. Check binding/port." << std::endl;
        mainMenu();
        return; 
    }

    auto shared_server = std::make_shared<uul::Server>();
    std::thread discovery_thread([shared_server](){
        shared_server->RunDiscoveryLoop();
    });
    discovery_thread.detach();

    if (!serverHandler.CreateServer(uul::DEFAULT_PORT)) {
        std::cerr << "FATAL: Failed to start TCP Game Server. Check binding/port." << std::endl;
        serverHandler.Cleanup();
        return; 
    }

    std::cout << "\nGame Server is now LIVE (TCP & UDP on port " << uul::DEFAULT_PORT << ")." << std::endl;
    std::cout << "Clients can now find and join your game." << std::endl;
    
    std::vector<std::unique_ptr<Player>> connected_players;
    const int MAX_CLIENTS = 5;

    while (connected_players.size() < MAX_CLIENTS) {
        std::cout << "\nWaiting for " << (MAX_CLIENTS - connected_players.size()) << " more player(s)..." << std::endl;
        
        SOCKET new_client_socket = serverHandler.AcceptClient(); 

        if (new_client_socket != INVALID_SOCKET) {
            connected_players.push_back(std::make_unique<Player>(new_client_socket));
            
            std::cout << "Client connected! Total players: " << connected_players.size() << "/" << MAX_CLIENTS << std::endl;
            
        } else {
            std::cerr << "Error accepting client. Exiting server loop." << std::endl;
            break; 
        }
    }
    
    std::cout << "\n--- Game session starting with " << connected_players.size() << " players! ---" << std::endl;
    // TODO: Game logic
    
    cliHandler.MultipleChoice({"Stop Server"}, "Press to end game and stop server.");
    
    serverHandler.Cleanup();
}

void joinGame(std::string ip) {
    if (serverHandler.ConnectClient(ip, uul::DEFAULT_PORT)) {
        std::cout << "Starting game session with server!" << std::endl;
    } else {
        std::cerr << "Failed to connect to " << ip << ". Returning to join menu." << std::endl;
        joinMenu();
    }
}

std::string joinOrCreate_Q() {
    std::string result = cliHandler.MultipleChoice({"Create Game", "Join Game"}, "Join or create game.");
    return result;
}

std::string joinOrSearch_Q() {
    std::string result = cliHandler.MultipleChoice({"Enter IP", "Search LAN", "Back"}, "Enter IP or search LAN games");
    return result;
}

std::string noGamesFound_Q() {
    std::string result = cliHandler.MultipleChoice({"Back"}, "No games found");
    return result;
}

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

    for (size_t i = 0; i < lanGames.size(); i++) {
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

            joinGame(selectedGameIP);
            while (true) {}
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing selection: " << selectedOption << ". Returning to join menu." << std::endl;
        joinMenu();
    }
}

void joinMenu() {
    std::system(CLEAR_SCREEN_CMD.c_str());
    std::string joinOrSearch = joinOrSearch_Q();

    if (joinOrSearch == "Enter IP") {
        enterIPMenu();
    } else if (joinOrSearch == "Search LAN") {
        selectGameMenu();
    } else if (joinOrSearch == "Back") {
        mainMenu();
    }
}

void enterIPMenu() {
    std::system(CLEAR_SCREEN_CMD.c_str());
    std::string ip;

    std::cout << "Press 'escape' to exit." << std::endl;
    std::cout << "Enter IP: ";
    std::cin >> ip;

    std::cout << "Attemtping to join game at: " << ip << std::endl;
    joinGame(ip);
    while (true) {}
}

void mainMenu() {
    std::system(CLEAR_SCREEN_CMD.c_str());
    std::string joinOrCreate = joinOrCreate_Q();
    
    if (joinOrCreate == "Join Game") {
        joinMenu();
    } else if (joinOrCreate == "Create Game") {
        createGame();
    }
}

int main() {
    cliHandler.EnableUTF8();
    mainMenu();
    return 0;
}