import random

def main():
    num = random.randint(1, 100)

    while (True):
        guessStr = input('Guess: ')

        try:
            guess = int(guessStr)
        except ValueError:
            print("Please enter a valid number")
            continue

        if guess > num:
            print('Lower')
        elif guess < num:
            print('Higher')
        else:
            print(f'You got it! The number was {num}')
            break

if __name__ == '__main__':
    main()