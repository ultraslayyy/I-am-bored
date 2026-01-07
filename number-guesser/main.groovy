def num = new Random().nextInt(100) + 1

while (true) {
    print "Guess: "
    def input = System.console().readLine()

    if (!input.isInteger()) {
        println "You must enter a valid number"
        continue
    }

    def guess = input.toInteger()

    if (guess > num) {
        println "Lower"
    } else if (guess < num) {
        println "Higher"
    } else {
        println "You got it! The number was $num"
        break
    }
}