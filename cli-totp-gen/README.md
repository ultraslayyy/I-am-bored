# CLI TOTP Generator
A simple and very basic CLI-based TOTP generator for npm.

## Using locally
Since this package isn't on npm, you'll have to build and setup locally. Here's how:
1. Install package globally to use. In npm you can install local packages for use globally, just as if it was any other npm package. You can do this through:
    ```sh
    npm install -g
    ```
    Make sure to run this command separately from the first install command.
2. **Done!** That's all you need to set it up. You can run:
    ```sh
    votp help
    ```
    to see a list of commands, and:
    ```sh
    votp help <command>
    ```
    or
    ```sh
    votp <command> -h
    ```
    to see a more detailed help description on a particular command.

## Commands currently not implemented
- bump
- export
- import
- list
- qr
- validate