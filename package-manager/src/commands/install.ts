import fs from 'fs-extra';
import path from 'path';
import { extractTarball } from './add';

export async function installPackages(argv: string[]) {
    const args = await parseInstallArgs(argv);
    console.log(args);
    
    const lockPath = typeof args['lock'] === 'string' ? args['lock'] : 'vex-lock.json';
    if (!fs.existsSync(lockPath)) {
        console.error('No vex-lock.json file found.');
        process.exit(1);
    }

    const lock = JSON.parse(fs.readFileSync(lockPath, 'utf-8'));
    const entries = Object.entries(lock) as [string, { version: string; resolved: string; }][];

    if (entries.length === 0) {
        console.log('Nothing to install.');
        return;
    }

    await fs.ensureDir('node_modules');

    for (const [pkg, meta] of entries) {
        const extractPath = path.join('node_modules', pkg);

        const res = await fetch(meta.resolved);
        if (!res.ok || !res.body) {
            console.error(`Failed to fetch ${pkg}@${meta.version}`);
            continue;
        }

        const buffer = Buffer.from(await res.arrayBuffer());
        await extractTarball(buffer, extractPath);

        console.log(`Installed ${pkg}@${meta.version}`);
    }

    console.log('All packages installed from vex-lock.json');
}

type ParsedFlags = Record<string, string | boolean>;

export async function parseInstallArgs(args: string[]): Promise<ParsedFlags> {
    const flags: ParsedFlags = {}

    let i = 0;
    while (i < args.length) {
        const arg = args[i];

        if (arg.startsWith('--')) {
            const [key, value] = arg.slice(2).split('=');
            if (value !== undefined) {
                flags[key] = value;
            } else if (args[i + 1] && !args[i + 1].startsWith('-')) {
                flags[key] = args[i + 1];
                i++;
            } else {
                flags[key] = true;
            }
        } else if (arg.startsWith('-') && arg.length > 1) {
            const chars = arg.slice(1).split('');

            for (let j = 0; j < chars.length; j++) {
                const char = chars[j];
                const isLast = j === chars.length - 1;
                const next = args[i + 1];

                if (isLast && next && !next.startsWith('-')) {
                    flags[char] = next;
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