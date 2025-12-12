#include <iostream>

int main() {
    srand(time(0));

    int num, guess;
    num = rand() % 101;

    while (true) {
        std::cout << "Guess: ";
        if (!(std::cin >> guess)) {
            std::cout << "Please enter a valid number" << std::endl;
            break;
        }

        if (guess > num) {
            std::cout << "Lower" << std::endl;
        } else if (guess < num) {
            std::cout << "Higher" << std::endl;
        } else {
            std::cout << "You got it! The number was " << num << std::endl;
            break; 
        }
    }
}