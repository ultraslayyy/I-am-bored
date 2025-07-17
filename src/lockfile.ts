import fs from 'fs';

export interface Lockfile {
    [pkg: string]: {
        version: string;
        resolved: string;
    }
}

const LOCKFILE_PATH = 'vex-lock.json';

export function updateLockfile(pkg: string, version: string, resolved: string) {
    let data: Lockfile = {}

    if (fs.existsSync(LOCKFILE_PATH)) {
        data = JSON.parse(fs.readFileSync(LOCKFILE_PATH, 'utf-8'));
    }
    data[pkg] = { version, resolved }

    fs.writeFileSync(LOCKFILE_PATH, JSON.stringify(data, null, 2));
}