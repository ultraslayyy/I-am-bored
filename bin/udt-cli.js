#!/usr/bin/env node
try {
    const { enableCompileCache } = require('node:module');
    enableCompileCache?.();
} catch {}

const UDT = require('../lib/cli.js');

(async () => {
    const argv = process.argv.slice(2);
    const udt = new UDT.default({ argv });

    if (argv.length === 0) {
        await udt.execute('help');
        return;
    }

    const cmd = argv[0];
    const args = argv.slice(1);

    await udt.execute(cmd, args);
})();