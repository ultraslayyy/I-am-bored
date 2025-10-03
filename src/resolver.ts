import path from 'node:path';
import fs from 'node:fs';
import { parseFile } from './parser.js';
import { astToIR } from './transform.js';
import type { IRModule } from './ir.js';

interface ModuleGraph {
  entries: string[];
  modules: Map<string, IRModule>;
  mergedModule: IRModule;
}

export async function bundleModuleGraph(entry: string, { bundle = true } = {}): Promise<ModuleGraph> {
    const modules = new Map<string, IRModule>();
    const visited = new Set<string>();

    function resolveImport(from: string, spec: string) {
        if (spec.startsWith('./') || spec.startsWith('../')) {
        let resolved = path.resolve(path.dirname(from), spec);
        if (fs.existsSync(resolved)) return resolved;
        if (fs.existsSync(resolved + '.js')) return resolved + '.js';
        if (fs.existsSync(path.join(resolved, 'index.js'))) return path.join(resolved, 'index.js');
        }
        throw new Error(`Cannot resolve import '${spec}' from ${from}`);
    }

    function crawl(filePath: string) {
        if (visited.has(filePath)) return;
        visited.add(filePath);

        const ast = parseFile(filePath);
        const ir = astToIR(ast, filePath);
        modules.set(filePath, ir);

        if (bundle) {
        for (const node of ir.body) {
            if (node.type === 'Import' && node.specifier.startsWith('./')) {
            const resolved = resolveImport(filePath, node.specifier);
            crawl(resolved);
            }
        }
        }
    }

    crawl(entry);

    const mergedBodies: IRModule['body'] = [];
    for (const [, mod] of modules) {
        mergedBodies.push(...mod.body);
    }

    const mergedModule: IRModule = { type: 'Module', path: entry, body: mergedBodies };

    return { entries: [entry], modules, mergedModule };
}
