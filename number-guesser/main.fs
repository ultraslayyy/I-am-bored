[<EntryPoint>]
let main _ =
    let num = Random().Next(0, 101)

    let rec loop () =
        printf "Guess: "

        Console.ReadLine()
        |> Int32.TryParse
        |> function
            | true, guess when guess < num ->
                printfn "Higher"
                loop()

            | true, guess when guess > num ->
                printfn "Lower"
                loop()

            | true, _ ->
                printfn $"You got it! The number was {num}"

            | false, _ ->
                printfn "Please enter a valid number"
                loop()

    loop()
    0