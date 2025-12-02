# Texas Hold Em

Just a simple CLI-based LAN multiplayer Texas Hold 'Em game

## Prerequisites
- Windows
- g++ *(Recommended through [MSYS2](https://code.visualstudio.com/docs/cpp/config-mingw#_installing-the-mingww64-toolchain))*
- *Optional*: ar *(if you want to build the A-EXE)*

## Building
Just run
```sh
build
```
in this directory. You can also build specific components:
```sh
# Builds just the DLL
build dll
# Builds just the EXE (DLL must exist)
build exe
# Builds the A static library file
build a
# Builds the EXE using the A static library instead of the DLL (.a must exist)
build a-exe
# Builds A and A-EXE
build a-full
# Clean up all non-source files
build clean
```

## Usage
Just run `texasholdem.exe` to play.

### Creating a game
When you hit `Create Game` on main menu, it turns that terminal instance into the game server. If you also want to play on the same computer, you'll have to run the executable again and join from that (keep the other open).

### Joining a game
Pretty self-explanatory tbh.
