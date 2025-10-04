process.stdin.setRawMode(true);
process.stdin.resume();
process.stdin.setEncoding('utf8');

const keysPressed = new Set<string>();

process.stdin.on('data', (key) => {
    const strKey = key.toString();
    if (strKey === '\u0003') process.exit(); // Escape key
    keysPressed.add(strKey);
    setTimeout(() => keysPressed.delete(strKey), 100);
});

function isKeyPressed(chip8Key: number) {
    const map: Record<number, string> = {
        0x0: '0', 0x1: '1', 0x2: '2', 0x3: '3',
        0x4: '4', 0x5: '5', 0x6: '6', 0x7: '7',
        0x8: '8', 0x9: '9', 0xA: 'a', 0xB: 'b',
        0xC: 'c', 0xD: 'd', 0xE: 'e', 0xF: 'f'
    }
    return keysPressed.has(map[chip8Key]);
}

export class CPU {
    memory: Uint8Array;
    V: Uint8Array;
    I: number;
    pc: number;
    stack: number[];
    sp: number;
    delayTimer: number;
    soundTimer: number;
    display: boolean[][];

    constructor() {
        this.memory = new Uint8Array(4096);
        this.V = new Uint8Array(16);
        this.I = 0;
        this.pc = 0x200;
        this.stack = [];
        this.sp = 0;
        this.delayTimer = 0;
        this.soundTimer = 0;
        this.display = Array.from({ length: 32 }, () => Array(64).fill(false));
    }

    loadProgram(program: Uint8Array) {
        this.memory.set(program, 0x200);
    }

    cycle() {
        const opcode = (this.memory[this.pc] << 8) | this.memory[this.pc + 1];
        this.pc += 2;
        this.executeOpcode(opcode);
    }

    executeOpcode(opcode: number) {
        const X = (opcode & 0x0F00) >> 8;
        const Y = (opcode & 0x00F0) >> 4;
        const N = opcode & 0x000F;
        const NN = opcode & 0x00FF;
        const NNN = opcode & 0x0FFF; 

        switch (opcode & 0xF000) {
            case 0x0000:
                if (opcode === 0x00E0) {
                    this.display.forEach(row => row.fill(false));
                } else if (opcode === 0x00EE) {
                    this.pc = this.stack.pop()!;
                }
                break;
            case 0x1000:
                this.pc = NNN;
                break;
            case 0x2000:
                this.stack.push(this.pc);
                this.pc = NNN;
                break;
            case 0x3000:
                if (this.V[X] === NN) this.pc += 2;
                break;
            case 0x4000:
                if (this.V[X] !== NN) this.pc += 2;
                break;
            case 0x5000:
                if (this.V[X] === this.V[Y]) this.pc += 2;
                break;
            case 0x6000:
                this.V[X] = NN;
                break;
            case 0x7000:
                this.V[X] = (this.V[X] + NN) & 0xFF;
                break;
            case 0x8000: 
                switch (opcode & 0x000F) {
                    case 0x0: this.V[X] = this.V[Y]; break;
                    case 0x1: this.V[X] |= this.V[Y]; break;
                    case 0x2: this.V[X] &= this.V[Y]; break;
                    case 0x3: this.V[X] ^= this.V[Y]; break;
                    case 0x4:
                        const sum = this.V[X] + this.V[Y];
                        this.V[0xF] = sum > 0xFF ? 1 : 0;
                        this.V[X] = sum & 0xFF;
                        break;
                    case 0x5:
                        this.V[0xF] = this.V[X] > this.V[Y] ? 1 : 0;
                        this.V[X] = (this.V[X] - this.V[Y]) & 0xFF;
                        break;
                    case 0x6:
                        this.V[0xF] = this.V[Y] > this.V[X] ? 1 : 0;
                        this.V[X] >>= 1;
                        break;
                    case 0x7:
                        this.V[0xF] = this.V[Y] > this.V[X] ? 1 : 0;
                        this.V[X] = (this.V[Y] - this.V[X]) & 0xFF;
                        break;
                    case 0xE: 
                        this.V[0xF] = this.V[Y] > this.V[X] ? 1 : 0;
                        this.V[X] = (this.V[X] << 1) & 0xFF;
                        break;
                }
                break;
            case 0x9000:
                if (this.V[X] !== this.V[Y]) this.pc += 2;
                break;
            case 0xA000:
                this.I = NNN;
                break;
            case 0xB000:
                this.pc = NNN + this.V[0];
                break;
            case 0xC000:
                this.V[X] = (Math.floor(Math.random() * 256)) & NN;
                break;
            case 0xD000:
                this.V[0xF] = 0;
                for (let row = 0; row < N; row++) {
                    const spriteByte = this.memory[this.I + row];
                    for (let col = 0; col < 8; col++) {
                        const mask = 0x80 >> col;
                        if ((spriteByte & mask) !== 0) {
                            const px = (this.V[X] + col) % 64;
                            const py = (this.V[Y] + row) % 32;
                            if (this.display[py][px]) this.V[0xF] = 1;
                            this.display[py][px] = this.display[py][px] !== true;
                        }
                    }
                }
                break;
            case 0xE000:
                if (NN === 0x9E && isKeyPressed(this.V[X])) {
                    this.pc += 2;
                } else if (NN === 0xA1 && !isKeyPressed(this.V[X])) {
                    this.pc += 2;
                }
                break;
            case 0xF000:
                switch (NN) {
                    case 0x07: this.V[X] = this.delayTimer; break;
                    case 0x0A:
                        this.V[X] = -1;
                        for (let i = 0; i < 16; i++) {
                            if (isKeyPressed(i)) {
                                this.V[X] = i;
                                break;
                            }
                        }
                        if (this.V[X] === -1) this.pc -= 2;
                        break;
                    case 0x15: this.delayTimer = this.V[X]; break;
                    case 0x18: this.soundTimer = this.V[X]; break;
                    case 0x1E: this.I = (this.I + this.V[X]) & 0xFFFF; break;
                    case 0x29: this.I = this.V[X] * 5; break;
                    case 0x33:
                        this.memory[this.I] = Math.floor(this.V[X] / 1000)
                        this.memory[this.I + 1] = Math.floor((this.V[X] & 100) / 10);
                        this.memory[this.I + 2] = this.V[X] & 10;
                        break;
                    case 0x55:
                        for (let i = 0; i <= X; i++) this.memory[this.I + i] = this.V[i];
                        break;
                    case 0x65:
                        for (let i = 0; i <= X; i++) this.V[i] = this.memory[this.I = i];
                        break;
                }
                break;
            default:
                console.log(`Unknown opcode: ${opcode.toString(16)}`);
        }
    }
}