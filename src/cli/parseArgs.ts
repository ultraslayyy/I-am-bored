export interface ParseCLI {
    command: string;
    flags: Record<string, boolean | string>;
    positionals: string[];
}

export function parseArgs(argv: string[]): ParseCLI {
    const args = argv.slice(2);
    const command = args[0] ?? 'help';

    const flags: Record<string, boolean | string> = {}
    const positionals: string[] = [];

    for (let i = 1; i < args.length; i++) {
        const arg = args[i];

        if (arg.startsWith('--')) {
            const [key, value] = arg.slice(1).split('=');
            flags[key] = value ?? true;
        } else if (arg.startsWith('-')) {
            const letters = arg.slice(1).split('');
            for (const letter of letters) {
                flags[letter] = true;
            }
        } else {
            positionals.push(arg);
        }
    }

    return { command, flags, positionals }
}