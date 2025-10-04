export async function multipleChoice(options: string[], question: string): Promise<string> {
    const grey = (text: string) => `\x1b[90m${text}\x1b[0m`;
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