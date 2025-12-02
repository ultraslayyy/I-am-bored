#include <iostream>

double convertTemp(double ftemp) {
    double ctemp = (ftemp - 32) * (5.0 / 9.0);
    return ctemp;
}

int main() {
    double ftemp;

    std::cout << "Temperature (F): ";
    std::cin >> ftemp;

    double ctemp = convertTemp(ftemp);

    std::cout << ctemp << std::endl;
}