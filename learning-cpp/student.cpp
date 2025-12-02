#include <cstdlib>
#include <ctime>
#include <iostream>

struct Student {
    std::string name;
    int id;
    float grade;
};

int main() {
    srand(time(0));

    Student classList[5];

    for (int i = 0; i < 5; i++) {
        classList[i].name = "";
        classList[i].id = 0;
        classList[i].grade = rand() % 101;
    }

    float sum = 0.0f, avg = 0.0f;
    for (int i = 0; i < 5; i++) {
        sum += classList[i].grade;
    }
    avg = sum / 5;

    std::cout << "Average grade is: " << avg << std::endl;
    return 0;
}