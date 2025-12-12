import scala.io.StdIn.readLine
import scala.util.Random

object Main extends App {
    val num = Random.nextInt(100) + 1
    
    while (true) {
        print("Guess: ")
        val inputStr = readLine()
        val guessTry = Try(inputStr.toInt)

        guessTry match {
            case scala.util.Success(guess) =>
                if (guess > num) {
                    println("Lower")
                } else if (guess < num) {
                    println("Higher")
                } else {
                    println(s"You got it! The number was $num")
                    System.exit(0)
                }
            case scala.util.Failure(_) =>
                println("Please enter a valid number)
        }
    }
}