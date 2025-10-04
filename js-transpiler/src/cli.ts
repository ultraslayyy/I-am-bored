#!/usr/bin/env node
import path from 'path';
import fs from 'fs';
import { emitModule as emitPython } from './emit_python.js';
import { emitModule as emitCoffee } from './emit_coffee.js';
import { bundleModuleGraph } from './resolver.js';

function usage() {
    console.log('Usage: node dist/cli.js <entry-file.js> [--out output.py]');
    process.exit(1);
}

async function main() {
    const argv = process.argv.slice(2);
    if (argv.length === 0) usage();

    const testIndex = argv.indexOf('--test');

    const entryIndex = argv.findIndex((arg) => !arg.startsWith('-'));
    if (entryIndex === -1) usage();

    const entry = testIndex > 0 ? path.resolve('tests', 'fixtures', argv[entryIndex]!) : path.resolve(argv[entryIndex]!);

    const langIndex = argv.indexOf('--lang');
    const lang = langIndex > 0 ? argv[langIndex + 1] : 'py';

    const outIndex = argv.indexOf('--out');
    const outFile = testIndex > 0 ? path.resolve('tests', 'fixtures', lang!, argv[outIndex + 1]!) : outIndex >= 0 ? path.resolve(argv[outIndex + 1]!) : path.resolve(process.cwd(), `out.${lang}`);

    const bundleIndex = argv.indexOf('--bundle');
    
    const graph = await bundleModuleGraph(entry, { bundle: bundleIndex !== -1 });
    const merged = graph.mergedModule;

    let code;
    if (lang === 'coffee') {
        code = emitCoffee(merged);
    } else {
        code = emitPython(merged, { includeRuntimeImport: true });
    }

    fs.writeFileSync(outFile, code, 'utf8');
    console.log(`✅ Wrote transpiled ${lang} to ${outFile}`);
}

main().catch((err) => {
    console.error(err);
    process.exit(1);
});
