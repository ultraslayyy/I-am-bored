let () =
    Random.self_init ();
    let num = Random.int 101 in

    let rec loop () =
        print_string "Guess: ";
        let input = read_line () in
        match int_of_string_opt input in
        | None -> print_endline "Please enter a valid number"; loop ()
        | Some guess ->
            if guess > num then (print_endline "Lower"; loop ())
            else if guess < num then (print_endline "Higher"; loop ())
            else Printf.printf "You got it! The number was %d\n" num
    in
    loop ()