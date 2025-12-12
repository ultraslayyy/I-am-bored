import Foundation

let num = Int.random(in: 1...100)

while true {
    print("Guess: ", terminator: "")
    
    guard let input = readLine(), !input.isEmpty else {
        print("Please enter a valid number")
        continue
    }
    
    guard let guess = Int(input) else {
        print("Please enter a valid number")
        continue
    }
    
    if guess > num {
        print("Lower")
    } else if guess < num {
        print("Higher")
    } else {
        print("You got it! The number was \(num)")
        break
    }
}