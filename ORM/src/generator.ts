import { loadConfig, type ResolvedConfig } from './config.js';
import { bundleRequire } from 'bundle-require';

function isEnum(v: any): v is { toSQL: () => string; name: string; values: string[] } {
    return (
        v &&
        typeof v === "object" &&
        typeof v.toSQL === "function" &&
        typeof v.name === "string" &&
        Array.isArray((v as any).values)
    );
}

function isTable(v: any): v is { toSQL: () => string; name: string; columns: Record<string, any> } {
    return (
        v &&
        typeof v === "object" &&
        typeof v.toSQL === "function" &&
        typeof v.name === "string" &&
        v.columns && typeof v.columns === "object"
    );
}

export async function generate(cwd = process.cwd(), explicitConfig?: string): Promise<string> {
    const cfg: ResolvedConfig = await loadConfig(cwd, explicitConfig);

    const { mod } = await bundleRequire({ filepath: cfg.schema , format: 'esm' });
    const schemaModule: any = (mod as any).default ?? mod;

    const exportsArray = Object.values(schemaModule);

    const parts: string[] = [];
    for (const v of exportsArray) if (isEnum(v)) parts.push((v as any).toSQL());
    for (const v of exportsArray) if (isTable(v)) parts.push((v as any).toSQL());

    return parts.join('\n\n') + (parts.length ? '\n' : '');
}