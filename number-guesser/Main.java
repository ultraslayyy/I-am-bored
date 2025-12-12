import java.util.Random;
import java.util.Scanner;

class NumberGuesser {
    public static void main(String[] args) {
        Random random = new Random();
        int num = random.nextInt(101);
        Scanner scanner = new Scanner(System.in);

        while (true) {
            System.out.print("Guess: ");

            if (!scanner.hasNextInt()) {
                System.out.println("Please enter a valid number");
                scanner.next();
                continue;
            }
            int guess = scanner.nextInt();

            if (guess > num) {
                System.out.println("Lower");
            } else if (guess < num) {
                System.out.println("Higher");
            } else {
                System.out.println("You got it! The number was " + num);
                break;
            }
        }

        scanner.close();
    }
}