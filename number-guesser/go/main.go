package main

import (
	"bufio"
	"fmt"
	"math/rand"
	"os"
	"strconv"
	"strings"
	"time"
)

func main() {
	source := rand.NewSource(time.Now().UnixNano())
	randomizer := rand.New(source)
	num := randomizer.Intn(100)

	reader := bufio.NewReader((os.Stdin))

	for {
		fmt.Printf("Guess: ")
		input, err := reader.ReadString('\n')

		if err != nil {
			fmt.Println("Please enter a valid number")
			continue
		}
		input = strings.TrimSpace(input)
		guess, err := strconv.Atoi(input)
		if err != nil {
			fmt.Println("Please enter a valid number")
			continue
		}

		if guess > num {
			fmt.Printf("Lower\n")
		} else if guess < num {
			fmt.Printf("Higher\n")
		} else {
			fmt.Printf("You got it! The number was %d", num)
			break
		}
	}
}
