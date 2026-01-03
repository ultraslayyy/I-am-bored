(defn guessing-game []
  (let [num (rand-int 101)]
    (println "Welcome to the guessing game!")
    (loop []
      (print "Guess: ")
      (flush)
      (let [input (read-line)
            guess (try (Integer/parseInt input) (catch Exception e nil))]
        (cond
          (nil? guess) (do (println "Please enter a valid number") (recur))
          (> guess num) (do (println "Lower") (recur))
          (< guess num) (do (println "Higher") (recur))
          :else (println "You got it! The number was" num))))))