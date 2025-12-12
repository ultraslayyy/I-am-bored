import kotlin.random.Random

fun main() {
    val num = Random.nextInt(0, 101)

    while (true) {
        printf("Guess: ")
        val input = readLine()
        val guess = input?.toIntOrNull()

        if (guess == null) {
            println("Please enter a valid number")
            continue
        }

        when {
            guess > num -> println("Lower")
            guess < num -> println("Higher")
            else -> {
                println("You got it! The number was $num")
                break
            }
        }
    }
}