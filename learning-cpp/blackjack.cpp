#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <random>
#include <chrono>

#include <windows.h>
#include <conio.h>

namespace ANSI {
    const std::string RESET = "\x1b[0m";
    const std::string GREY = "\x1b[90m";
    const std::string CYAN = "\x1b[36m";
    const std::string GREEN = "\x1b[32m";
    const std::string RED = "\x1b[31m";
    const std::string YELLOW = "\x1b[33m";
    const std::string MAGENTA = "\x1b[35m";
    const std::string CLEAR_LINE = "\x1b[2K";
    
    std::string moveUp(int n = 1) {
        return "\x1b[" + std::to_string(n) + "A";
    }
}

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

void goToXY(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(hConsole, coord);
}

COORD getCursorPosition() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        return csbi.dwCursorPosition;
    }
    return {0,0};
}

std::string multipleChoice(const std::vector<std::string>& options, const std::string& question) {
    if (options.empty()) return "";

    int selected = 0;
    COORD startPos = getCursorPosition();

    auto render = [&]() {
        goToXY(0, startPos.Y);
        std::cout << ANSI::CLEAR_LINE << question << "\n";

        for (int i = 0; i < options.size(); i++) {
            std::cout << ANSI::CLEAR_LINE;
            const std::string prefix = (selected == i) ? ANSI::CYAN + "> " + ANSI::RESET : "  ";
            std::cout << prefix << options[i] << "\n";
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

std::vector<std::string> deck;
std::default_random_engine rng(std::chrono::system_clock::now().time_since_epoch().count());

void genDeck() {
    deck.clear();
    std::vector<std::string> suits = {"\u2665", "\u2660", "\u2666", "\u2663"};
    std::vector<std::string> ranks = {"A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"};

    for (const auto& suit : suits) {
        for (const auto& rank : ranks) {
            deck.push_back(rank + suit);
        }
    }
    std::shuffle(deck.begin(), deck.end(), rng);
}

int getCardValue(const std::string& card) {
    if (card.length() >= 2 && card.substr(0, 2) == "10") {
        return 10;
    }
    
    std::string rank = card.substr(0, 1);
    
    if (rank == "J" || rank == "Q" || rank == "K") return 10;
    if (rank == "A") return 11;
    
    return std::stoi(rank);
}

int calculateHandValue(const std::vector<std::string>& hand) {
    int value = 0;
    int aces = 0;

    for (const auto& card : hand) {
        std::string rank_str;
        if (card.length() >= 2 && card.substr(0, 2) == "10") {
            rank_str = "10";
        } else {
            rank_str = card.substr(0, 1);
        }
        
        if (rank_str == "A") {
            aces++;
            value += 11;
        } else {
            value += getCardValue(card);
        }
    }

    while (value > 21 && aces > 0) {
        value -= 10;
        aces--;
    }

    return value;
}

std::string genCard() {
    if (deck.empty()) {
        throw std::runtime_error("Deck is empty");
    }
    std::string card = deck.back();
    deck.pop_back();
    return card;
}

bool hitStandQ_Interactive() {
    std::vector<std::string> options = {"Hit", "Stand"};
    std::string result = multipleChoice(options, "Your move:");

    return result == "Hit";
}

std::string formatHand(const std::vector<std::string>& hand) {
    std::string result;
    for (size_t i = 0; i < hand.size(); ++i) {
        result += hand[i];
        if (i < hand.size() - 1) {
            result += ", ";
        }
    }
    return result;
}

void mainGame() {
    genDeck();

    std::vector<std::string> playerHand = {genCard(), genCard()};
    std::vector<std::string> dealerHand = {genCard(), genCard()};

    int playerValue = calculateHandValue(playerHand);
    int dealerValue = calculateHandValue(dealerHand);

    std::cout << "------------------------------------------\n";
    std::cout << "Your hand: " << formatHand(playerHand) << " = " << playerValue << "\n";
    std::cout << "Dealer shows: " << dealerHand[0] << "\n";
    std::cout << "------------------------------------------\n";

    std::cout << ANSI::CYAN << "YOUR TURN" << ANSI::RESET << "\n";
    while (playerValue < 21) {
        bool hit = hitStandQ_Interactive();
        
        if (hit) {
             std::cout << "\n"; 
        }
        
        if (!hit) break;

        std::string card = genCard();
        playerHand.push_back(card);
        playerValue = calculateHandValue(playerHand);
        
        std::cout << "You drew " << card << ".\n";
        std::cout << "Your hand is now " << formatHand(playerHand) << " = " << playerValue << "\n\n";
    }

    if (playerValue > 21) {
        std::cout << ANSI::RED << "BUST! You lose." << ANSI::RESET << "\n";
        return;
    }

    std::cout << ANSI::MAGENTA << "DEALER'S TURN" << ANSI::RESET << "\n";
    std::cout << "Dealer's full hand: " << formatHand(dealerHand) << " = " << dealerValue << "\n";
    while (dealerValue < 17) {
        Sleep(1000);
        std::string card = genCard();
        dealerHand.push_back(card);
        dealerValue = calculateHandValue(dealerHand);
        
        std::cout << "Dealer draws " << card << ".\n";
        std::cout << "Dealer hand: " << formatHand(dealerHand) << " = " << dealerValue << "\n";
    }
    std::cout << "------------------------------------------\n";

    if (dealerValue > 21) {
        std::cout << ANSI::GREEN << "Dealer busts! You win!" << ANSI::RESET << "\n";
    } else if (playerValue > dealerValue) {
        std::cout << ANSI::GREEN << "You win with a higher total!" << ANSI::RESET << "\n";
    } else if (playerValue < dealerValue) {
        std::cout << ANSI::RED << "Dealer wins." << ANSI::RESET << "\n";
    } else {
        std::cout << ANSI::YELLOW << "Push (Tie)." << ANSI::RESET << "\n";
    }
}

int main() {
    system("chcp 65001");

    DWORD mode;
    GetConsoleMode(hConsole, &mode);
    SetConsoleMode(hConsole, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    mainGame();

    std::cout << "------------------------------------------\n";
    std::cout << "This is what I spent English making in C++" << std::endl;

    Sleep(10000);
    return 0;
}

// g++ -o blackjack.exe blackjack.cpp -Os -s -ffunction-sections -fdata-sections -Wl,--gc-sections -static-libgcc