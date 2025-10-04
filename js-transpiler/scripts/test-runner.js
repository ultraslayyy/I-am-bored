import fs from 'fs';
import path from 'path';
import { execSync } from 'child_process';

const FIXTURES_DIR = path.resolve(process.cwd(), 'tests/fixtures');

function getJsFiles(dir) {
    const entries = fs.readdirSync(dir, { withFileTypes: true });
    let files = [];
    for (const entry of entries) {
        const fullPath = path.join(dir, entry.name);
        if (entry.isDirectory()) {
            files.push(...getJsFiles(fullPath));
        } else if (entry.isFile() && entry.name.endsWith('.js')) {
            files.push(entry.name);
        }
    }
    return files;
}

function transpileFile(jsFile) {
    const pyFile = jsFile.replace(/\.js$/, '.py');
    const coffeeFile = jsFile.replace(/\.js$/, '.coffee');
    try {
        execSync(`node dist/cli.js "${jsFile}" --out "${pyFile}" --lang py --test`, { stdio: 'inherit' });
        execSync(`node dist/cli.js "${jsFile}" --out "${coffeeFile}" --lang coffee --test`, { stdio: 'inherit' });
    } catch (err) {
        console.error(`❌ Failed to transpile ${jsFile}`);
        console.error(err);
    }
}

function runAllTests() {
    const files = getJsFiles(FIXTURES_DIR);
    for (const file of files) {
        transpileFile(file);
    }
}

runAllTests();
