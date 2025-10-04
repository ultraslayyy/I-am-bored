#!/usr/bin/env node
try {
    const { enableCompileCache } = require('node:module');
    enableCompileCache?.();
} catch {}

const Votp = require('../lib/cli.js');

(async () => {
    const argv = process.argv.slice(2);
    const votp = new Votp.default({ argv });

    if (argv.length === 0) {
        console.log('Please specify a command.');
        return;
    }

    const cmd = argv[0];
    const args = argv.slice(1);

    await votp.execute(cmd, args);
})();