# CHIP 8 CPU in TypeScript
This is a simple CHIP 8 CPU emulator that I built in TypeScript.

## Creating a program
To create a program, I have a simple example that detects keyboard inputs in [index.ts](src/index.ts). But here's how to do it from scratch in just 10 small steps.
1. Install dependencies. Depending on what JavaScript package manager you use this may look slightly different, but for npm it's
    ```sh
    npm install
    ```
2. First import the CPU using:
    ```ts
    import { CPU } from './cpu.js';
    ```
3. Initialise an instance of the CPU:
    ```ts
    const cpu = new CPU();
    ```
4. Create your program. You do this by create a new variable, which is a `Uint8Array`, like:
    ```ts
    const program = new Uint8Array([]);
    ```
5. Fill the Uint8Array with instructions. If you need to learn more then just google it.
6. Load the program, via
    ```ts
    cpu.loadProgram(program);
    ```
7. Possibly add other code, like my input detection code I have in the example.
8. Run through some kind of loop, that triggers `cpu.cycle()` somewhere in it, which will cycle to the next cpu instruction in the list provided in the program variable.
9. Build and run. Do this through two commands:
    ```sh
    npm run build
    npm start
    ```
    > [!NOTE]
    > If you want to test without rebuilding, you can always use:
    > ```sh
    > npm run dev
    > ```
    > Which will run the TypeScript file.
10. **Done!**
    You know have your own CHIP 8 program.