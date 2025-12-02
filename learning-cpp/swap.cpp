#include <iostream>

void swapValues(int& a, int& b) {
    int c;
    c = a;
    a = b;
    b = c;
}

int main() {
    int a = 5;
    int b = 10;

    std::cout << "Variables: " << a << ", " << b << std::endl;
    swapValues(a, b);
    std::cout << "Variables: " << a << ", " << b << std::endl;
    return 0;
}