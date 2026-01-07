## Current Languages
The current languages I've written this in are:
- [Assembly (x86 NASM)](./main.asm)
- [Bourne Shell](./main.sh)
- [C](./main.c)
- [C#](./c#/Program.cs)
- [C++](./main.cpp)
- [Clojure](./main.clj)
- [COBOL](./main.cbl)
- [CoffeeScript](./main.coffee)
- [Dart](./main.dart)
- [Dartmouth BASIC](./main.bas)
- [DOS Batch](./main.bat)
- [Elixir](./main.exs)
- [Fortran](./main.f90)
- [Go](./go/main.go)
- [Groovy](./main.groovy)
- [Haskell](./main.hs)
- [HolyC](./main.HC)
- [Java](./Main.java)
- [JavaScript](./main.js)
- [Julia](./main.jl)
- [Kotlin](./Main.kt)
- [Lisp](./main.lisp)
- [Lua](./main.lua)
- [Objective-C](./main.m)
- [Perl](./main.pl)
- [PHP](./main.php)
- [PL/SQL](./main.pls)
- [Powershell](./main.ps1)
- [Prolog](./main.pro)
- [Python](./main.py)
- [R](./main.r)
- [Ruby](./main.rb)
- [Rust](./rust/src/main.rs)
- [Scala](./main.scala)
- [Swift](./main.swift)
- [TypeScript](./typescript/main.ts)
- [Zig](./main.zig)

### Todo
- Visual Basic
- F#
- Nim
- Crystal
- OCaml
- VHDL or Verilog

## Rules
- Non-built-in dependencies are **prohibited**
- Structure (alot of languages don't care about this, so I can't for some)
  - Declare **one** `main()` function, which must be the sole entrypoint of the code, and run at the start of the file.
  - Must be in this structure:
    ```
    [package/namespace declaration (if required)]

    [imports (if required)]

    [class declaration (if required)]
    [function declaration (main)]
      [random number declaration and assignment (num)]

      [while loop]
        [accept input]

        [input validation]
        
        [if statements (non-inline)]

    [main() function call (if required)]
    ```
    - Outside of this, empty lines are **not** allowed, besides between `num` and `guess` declaration, **if** `num` *declaration & assignment* takes 3 or more lines.
    - If the input is invalid (`[input validation]`), the message returned must be: `Please enter a valid number`, and then reprompt (`continue`). If validation is done inline (e.g. Swift's `guard`), this can be skipped
  - Only **one** helper function is allowed, and must be placed above the declaration of `main()`. That helper function is **only allowed to contain random number generation logic**, if complicated in that language (Haskell is an exception, where it's required for the loop). This is discouraged as it increases line count. Always look for the easiest solution, as to avoid helpers.
- Input/Output
  - Input prompt must explicitly ask for an input, with the prompt `Guess: `
  - Output feedback must be one of:
    - `Higher` — guess is lower than num
    - `Lower` — guess is higher than num
    - `You got it! The number was $num` — guess is the same as num
- Random Number
  - `num` must be in the range `1-100` inclusive
  - Must be generated **once** at the start of main()
- Variables
  - Variables must be named `num` and `guess`
  - No global variables (must be scoped in `main()`)
- Style
  - Only **one** statement per line (no semicolon chains). The cases exempt from this are:
    - Multi assignments (e.g. `int num, guess`)
    - `guard` in Swift
  - Braces' content **must** be dropped to another line, not inline (no `{content}`, only `{\ncontent\n}`)
  - If statements must be in the order:
    - `guess > num`
    - `guess < num`
    - `else`
- Random Seed
  - If the language allows seeding the generator, seed it with current time or system entropy.
- Termination
  - Program must exit **only** after a successful guess
  - No extra prompts or loops after a correct guess
