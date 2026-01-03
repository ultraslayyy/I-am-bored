defmodule GuessingGame do
  def play do
    num = :rand.uniform(101) - 1
    game_loop(num)
  end

  defp game_loop(num) do
    input = IO.gets("Guess: ) |> String.trim()
    case Integer.parse(input) do
      {guess, _} =>
        check_guess(guess, num)

      :error ->
        IO.puts("Please enter a valid number")
        game_loop(num)
    end
  end

  defp check_guess(guess, num) when guess > num do
    IO.puts("Lower")
    game_loop(num)
  end

    defp check_guess(guess, num) when guess < num do
    IO.puts("Higher")
    game_loop(num)
  end

  defp check_guess(_guess, num) do
    IO.puts("You got it! The number was #{num}")
  end
end

GuessingGame.play()