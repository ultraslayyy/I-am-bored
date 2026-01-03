program guessing_game
    implicit none
    integer :: num, guess
    real :: random_val
    integer :: iostat
    call random_seed()
    call random_number(random_val)
    num = int(random_val * 101)

    do
        write(*, '(A)', advance='no') "Guess: "
        read(*, *, iostat=iostat) guess

        if (iostat /= 0) then
            print *, "Please enter a valid number"
            cycle
        end if

        if (guess > num) then
            print *, "Lower"
        else if (guess < num) then
            print *, "Higher"
        else
            print *, "You got it! The number was ", num
            exit
        end if
    end do
end program guessing_game