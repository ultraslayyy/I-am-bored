#include <iostream>
#include <algorithm>

char validOps[4] = { '+', '-', '*', '/' };

int main() {
    double n1, n2, a;
    char o;

    std::cout << "Number 1: ";
    std::cin >> n1;
    std::cout << "Operation: ";
    std::cin >> o;
    if (std::find(std::begin(validOps), std::end(validOps), o) == std::end(validOps)) {
        std::cout << "Invalid operator" << std::endl;
        return 1;
    }
    std::cout << "Number 2: ";
    std::cin >> n2;

    switch (o) {
        case '+':
            a = n1 + n2;
            break;
        case '-':
            a = n1 - n2;
            break;
        case '*':
            a = n1 * n2;
            break;
        case '/':
            if (n2 == 0) {
                a = 0;
            } else {
                a = n1 / n2;
            }
            break;
        default:
            return 1;
    }

    std::cout << a << std::endl;
    return 0;
}