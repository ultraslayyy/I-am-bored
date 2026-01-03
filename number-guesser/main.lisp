(defun guessing-game ()
  (let ((num (random 101))
        (guess nil))
    (format t "Welcome to the guessing game!~%")
    (loop
      (format t "Guess: ")
      (finish-output)
      (setf guess (read nil))

      (cond
        ((not (numberp guess)) (format t "Please enter a valid number~%"))
        ((> guess num) (format t "Lower~%"))
        ((< guess num) (format t "Higher~%"))
        (t (return (format t "You got it! The number was ~a~%" num)))))))