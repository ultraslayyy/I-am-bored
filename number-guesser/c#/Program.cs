namespace c_;

class Program
{
    static void Main()
    {
        Random random = new();
        int num = random.Next(0, 101);

        while (true)
        {
            Console.Write("Guess: ");
            string? input = Console.ReadLine();

            if (!int.TryParse(input, out int guess))
            {
                Console.WriteLine("Please enter a valid number");
                continue;
            }

            if (guess > num)
            {
                Console.WriteLine("Lower");
            }
            else if (guess < num)
            {
                Console.WriteLine("Higher");
            }
            else
            {
                Console.WriteLine($"You got it! The number was {num}");
                break;
            }
        }
    }
}
