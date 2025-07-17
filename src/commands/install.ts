import fs from 'fs-extra';
import path from 'path';
import { extractTarball } from './add';

export async function installPackages() {
    const lockPath = 'vex-lock.json';
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