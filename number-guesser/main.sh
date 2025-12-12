#!/bin/bash

num=$(($RANDOM % 100 + 1))

while true; do
    read -p "Guess: " guess

    if [ -z "$guess" ]; then
        echo "Please enter a valid number"
        continue
    fi

    if [[ ! "$guess" =~ ^[0-9]+$ ]]; then
        echo "Please enter a valid number"
        continue
    fi

    if (( guess < num )); then
        echo "Higher"
    elif (( guess > num )); then
        echo "Lower"
    else
        echo "You got it! The number was $num"
        break
    fi
done
