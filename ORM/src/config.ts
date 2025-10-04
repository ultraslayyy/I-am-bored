import path from 'node:path';
import fs from 'node:fs/promises';
import { pathToFileURL } from 'node:url';
import { bundleRequire } from 'bundle-require';

export type UserConfig = {
    schema: string;
    out?: string;
    dialect?: 'postgres';
}

export type ResolvedConfig = {
    schema: string;
    out: string;
    dialect: 'postgres';
    configFile: string;
}

export function defineConfig(config: UserConfig): UserConfig {
    return config;
}

const CONFIG_BASENAME = 'orm';

const CANDIDATES = [
    `${CONFIG_BASENAME}.config.ts`,
    `${CONFIG_BASENAME}.config.mts`,
    `${CONFIG_BASENAME}.config.cts`,
    `${CONFIG_BASENAME}.config.js`,
    `${CONFIG_BASENAME}.config.mjs`,
    `${CONFIG_BASENAME}.config.cjs`,
    `${CONFIG_BASENAME}.config.json`
];

async function findConfig(cwd: string, explicit?: string): Promise<string> {
    if (explicit) {
        const p = path.resolve(cwd, explicit);
        await fs.access(p);
        return p;
    }
    for (const name of CANDIDATES) {
        const p = path.resolve(cwd, name);
        try {
            await fs.access(p);
            return p;
        } catch {}
    }
    throw new Error(`No config file found. Looked for: ${CANDIDATES.join(', ')} in ${cwd}`);
}

export async function loadConfig(cwd = process.cwd(), explicitPath?: string): Promise<ResolvedConfig> {
    const configFile = await findConfig(cwd, explicitPath);

    let raw: any;
    if (/\.(ts|mts|cts)$/.test(configFile)) {
        const { mod } = await bundleRequire({ filepath: configFile, format: 'esm' });
        raw = (mod as any).default ?? mod;
    } else if (/\.json$/.test(configFile)) {
        const url = pathToFileURL(configFile).href;
        const mod = await import(url);
    } else {
        const url = pathToFileURL(configFile).href;
        const mod = await import(url);
        raw = (mod as any).default ?? mod;
    }

    const cfg = typeof raw === 'function' ? await raw() : raw;
    if (!cfg?.schema) {
        throw new Error(`Config at ${configFile} must export { schema: string } (use defineConfig for types)`);
    }

    const resolved: ResolvedConfig = {
        schema: path.resolve(cwd, cfg.schema),
        out: path.resolve(cwd, cfg.out ?? 'schema.sql'),
        dialect: 'postgres',
        configFile
    }

    return resolved;
}