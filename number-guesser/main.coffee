readline = require 'node:readline'

main = ->
  rl = readline.createInterface
    input: process.stdin
    output: process.stdout

  num = Math.floor(Math.random() * 100) + 1

  ask = ->
    new Promise (resolve) ->
      rl.question 'Guess: ', resolve

  while true
    guess = await ask()

    if isNaN Number guess
      console.log 'You must enter a valid number'
      continue

    if guess > num
      console.log 'Lower'
    else if guess < num
      console.log 'Higher'
    else
      console.log "You got it! The number was #{num}"
      rl.close()
      break

main()
