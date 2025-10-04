import path from 'path';
import fs from 'fs';
import { defaultPackageJson } from '../cli/defaults';
import readline from 'readline';

type ParsedFlags = Record<string, string | boolean>;

async function parseInitArgs(args: string[]) {
    const flags: ParsedFlags = {};

    let i = 0;
    while (i < args.length) {
        const arg = args[i];

        if (arg.startsWith('--')) {
            const key = arg.slice(2);
            flags[key] = true;
        } else if (arg.startsWith('-') && arg.length > 1) {
            const chars = arg.slice(1).split('');

            for (let j = 0; j < chars.length; j++) {
                const char = chars[j];
                const isLast = j === chars.length - 1;
                const next = args[i + 1];

                if (isLast && next && !next.startsWith('-')) {
                    flags[char];
                    i++;
                } else {
                    flags[char] = true;
                }
            }
        }

        i++;
    }

    return flags;
}

export async function init(argv: string[], override?: boolean) {
    if (fs.existsSync('package.json') && !override) {
        console.warn('A package.json already exists at this location.');
        const userChoice = await askMultiple(['Cancel', 'Override'], 'Do you wish to override the existing package.json?');

        if (userChoice === 'Override') {
            await init(argv, true);
            return;
        } else return;
    }
    const args = await parseInitArgs(argv);

    const dirName = path.basename(process.cwd());
    let json;

    if (args['y'] || args['yes']) {
        const obj = JSON.parse(defaultPackageJson({ name: dirName.toLowerCase() }));
        json = JSON.stringify(obj, null, 2);
    } else {
        const name = await askQuestion('Package name', dirName);
        const version = await askQuestion('Version', '1.0.0');
        const desc = await askQuestion('Description');
        const main = await askQuestion('Entry point', 'index.js');
        const script = await askQuestion('Test script');
        const keywords = await askKeywords();
        const author = await askQuestion('Author');
        const license = await askQuestion('License', 'ISC');
        const moduleType = await askMultiple(['commonjs', 'module'], 'Type: ');

        const obj = JSON.parse(defaultPackageJson({
            name: name.toLowerCase(),
            version,
            desc,
            main,
            script,
            keywords,
            author,
            license,
            type: moduleType as 'module' | 'commonjs'
        }));
        json = JSON.stringify(obj, null, 2);
    }

    fs.writeFileSync('package.json', json);
    console.log(`Successfully ${override ? 'Overid' : 'Created'} package.json`);
    console.log(json);
}

const grey = (text: string) => `\x1b[90m${text}\x1b[0m`;

function askQuestion(query: string, defaultValue?: string): Promise<string> {
    const rl = readline.createInterface({
        input: process.stdin,
        output: process.stdout
    });

    const fullPrompt = defaultValue ? `${query} ${grey(`(${defaultValue})`)}: ` : `${query}: `;

    return new Promise(resolve => {
        rl.question(fullPrompt, answer => {
            rl.close();
            resolve(answer.trim() || (defaultValue === 'comma separated' ? undefined : defaultValue) || '');
        });
    });
}

async function askKeywords(): Promise<string[]> {
    const input = await askQuestion('Keywords', 'comma separated');
    return input.split(',').map(s => s.trim()).filter(s => s.length > 0);
}

function askMultiple(options: string[], question: string): Promise<string> {
    const helpText = `${grey(`(Use Up & Down to select, Space or Enter to confirm)`)}`;

    return new Promise(resolve => {
        let selected = 0;

        const clearLine = () => process.stdout.write('\x1b[2K');
        const moveUp = (n = 1) => process.stdout.write(`\x1b[${n}A`);

        const render = () => {
            moveUp(options.length + 1);
            clearLine();
            console.log(question);
            for (let i = 0; i < options.length; i++) {
                clearLine();
                const prefix = selected === i ? '\x1b[36m> \x1b[0m' : '  ';
                console.log(`${prefix}${options[i]}`);
            }
        };

        const handleKeyPress = (chunk: Buffer) => {
            const key = chunk.toString();

            if (chunk[0] === 3) process.exit();

            if (key === '\u001b[A') {
                selected = (selected - 1 + options.length) % options.length;
                render();
            } else if (key === '\u001b[B') {
                selected = (selected + 1) % options.length;
                render();
            } else if (key === ' ' || key === '\r') {
                process.stdin.setRawMode(false);
                process.stdin.pause();
                process.stdin.removeListener('data', handleKeyPress);
                console.log();
                resolve(options[selected]);
            }
        };

        console.log(question);
        options.forEach((opt, i) => {
            const prefix = selected === i ? '\x1b[36m> \x1b[0m' : '  ';
            console.log(`${prefix}${opt}`);
        });

        process.stdin.setRawMode(true);
        process.stdin.resume();
        process.stdin.on('data', handleKeyPress);
    });
}