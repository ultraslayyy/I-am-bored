#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    srand(time(0));

    int num, guess;
    num = rand() % 101;
    char input[100];

    while (1) {
        printf("Guess: ");
        if (sscanf(input, "%d", &guess) != 1) {
            printf("Please enter a valid number\n");
            break;
        }

        if (guess > num) {
            printf("Lower\n");
        } else if (guess < num) {
            printf("Higher\n");
        } else {
            printf("You got it! The number was %d", num);
            break;
        }
    }
}