#!/usr/bin/env node
import fs from 'node:fs';
import path from 'node:path';
import { generate } from './generator.js';
import { loadConfig } from './config.js';

async function run() {
    const args = process.argv.slice(2);
    const configIdx = args.indexOf('--config');
    const configPath = configIdx >=0 ? args[configIdx + 1] : undefined;

    const sql = await generate(process.cwd(), configPath);
    const cfg = await loadConfig(process.cwd(), configPath);

    fs.mkdirSync(path.dirname(cfg.out), { recursive: true });
    fs.writeFileSync(cfg.out, sql);
    console.log(`SQL written to ${cfg.out}`);
}

run().catch((e) => {
    console.error(e.message || e);
    process.exit(1);
});