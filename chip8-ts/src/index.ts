import { CPU } from './cpu.js';

const cpu = new CPU();

const testProgram = new Uint8Array([
    0xF0, 0x0A
]);

cpu.loadProgram(testProgram);

process.stdin.setRawMode(true);
process.stdin.resume();
process.stdin.setEncoding('utf8');

const keysPressed = new Set<string>();

process.stdin.on('data', (rawkey) => {
    const key = rawkey.toString();
    if (key === '\u0003') process.exit();
    keysPressed.add(key);
    setTimeout(() => keysPressed.delete(key), 100);
});

function isKeyPressed(chip8Key: number) {
    const map: Record<number, string> = {
        0x0: '0', 0x1: '1', 0x2: '2', 0x3: '3',
        0x4: '4', 0x5: '5', 0x6: '6', 0x7: '7',
        0x8: '8', 0x9: '9', 0xA: 'a', 0xB: 'b',
        0xC: 'c', 0xD: 'd', 0xE: 'e', 0xF: 'f'
    };
    return keysPressed.has(map[chip8Key]);
}

function loop() {
    cpu.cycle();

    console.clear();
    console.log('Press a key (0-9, a-f)');
    console.log('V0 =', cpu.V[0]);
    console.log('Keys currently pressed:', Array.from(keysPressed).join(', '));

    setTimeout(loop, 100);
}

loop();

(global as any).isKeyPressed = isKeyPressed;
