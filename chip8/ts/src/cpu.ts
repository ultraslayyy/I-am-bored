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

const FONT = new Uint8Array([
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
]);

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
    stack: Uint16Array;
    sp: number;
    delayTimer: number;
    soundTimer: number;
    display: Uint8Array;

    constructor() {
        this.memory = new Uint8Array(4096);
        this.V = new Uint8Array(16);
        this.I = 0;
        this.pc = 0x200;
        this.stack = new Uint16Array(16);
        this.sp = 0;
        this.delayTimer = 0;
        this.soundTimer = 0;
        this.display = new Uint8Array(64 * 32);

        this.memory.set(FONT, 0);

        setInterval(() => {
            if (this.delayTimer > 0) this.delayTimer--;
            if (this.soundTimer > 0) this.soundTimer--;
        }, 1000 / 60);
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
                    this.display.fill(0);
                } else if (opcode === 0x00EE) {
                    if (this.sp === 0) {
                        throw new Error('Stack underflow');
                    }

                    this.pc = this.stack[--this.sp];
                }
                break;
            case 0x1000:
                this.pc = NNN;
                break;
            case 0x2000:
                if (this.sp >= 16) {
                    throw new Error('Stack overflow');
                }

                this.stack[this.sp++] = this.pc;
                this.pc = NNN;
                break;
            case 0x3000:
                if (this.V[X] === NN) this.pc += 2;
                break;
            case 0x4000:
                if (this.V[X] !== NN) this.pc += 2;
                break;
            case 0x5000:
                if (N !== 0) {
                    throw new Error(`Unknown opcode 0x${opcode.toString(16).padStart(4, '0')}`);
                }

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
                    case 0x4: {
                        const sum = this.V[X] + this.V[Y];
                        this.V[0xF] = sum > 0xFF ? 1 : 0;
                        this.V[X] = sum & 0xFF;
                        break;
                    }
                    case 0x5:
                        this.V[0xF] = this.V[X] >= this.V[Y] ? 1 : 0;
                        this.V[X] = (this.V[X] - this.V[Y]) & 0xFF;
                        break;
                    case 0x6:
                        this.V[0xF] = this.V[X] & 1;
                        this.V[X] >>= 1;
                        break;
                    case 0x7:
                        this.V[0xF] = this.V[Y] >= this.V[X] ? 1 : 0;
                        this.V[X] = (this.V[Y] - this.V[X]) & 0xFF;
                        break;
                    case 0xE: 
                        this.V[0xF] = (this.V[X] >> 7) & 1;
                        this.V[X] = (this.V[X] << 1) & 0xFF;
                        break;
                    default:
                        throw new Error(`Unknown opcode 0x${opcode.toString(16).padStart(4, '0')}`);
                }
                break;
            case 0x9000:
                if (N !== 0) {
                    throw new Error(`Unknown opcode 0x${opcode.toString(16).padStart(4, '0')}`);
                }

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
                            const idx = py * 64 + px;
                            if (this.display[idx]) this.V[0xF] = 1;
                            this.display[idx] ^= 1;
                        }
                    }
                }
                break;
            case 0xE000:
                if (NN === 0x9E && isKeyPressed(this.V[X])) {
                    this.pc += 2;
                } else if (NN === 0xA1 && !isKeyPressed(this.V[X])) {
                    this.pc += 2;
                } else {
                    throw new Error(`Unknown opcode 0x${opcode.toString(16).padStart(4, '0')}`);
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
                    case 0x33: {
                        const value = this.V[X];

                        this.memory[this.I] = Math.floor(value / 100);
                        this.memory[this.I + 1] = Math.floor((value % 100) / 10);
                        this.memory[this.I + 2] = value % 10;
                        break;
                    }
                    case 0x55:
                        for (let i = 0; i <= X; i++) this.memory[this.I + i] = this.V[i];
                        break;
                    case 0x65:
                        for (let i = 0; i <= X; i++) this.V[i] = this.memory[this.I + i];
                        break;
                    default:
                        throw new Error(`Unknown opcode 0x${opcode.toString(16).padStart(4, '0')}`);
                }
                break;
            default:
                throw new Error(`Unknown opcode: 0x${opcode.toString(16).padStart(4, '0')}`);
        }
    }
}