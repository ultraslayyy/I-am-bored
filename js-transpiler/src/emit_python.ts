import type {
    IRNode,
    IRModule,
    IRFunction,
    IRVarDecl,
    IRAssign,
    IRIf,
    IRReturn,
    IRExprStmt,
    IRCall,
    IRLiteral,
    IRIdentifier,
    IRBinary,
    IRArrowFunction,
    IRImport,
    IRIIFE,
    IRExpr
} from './ir.js';

let emitOptions: EmitOptions = {}

interface EmitOptions {
    indent?: string;
    includeRuntimeImport?: boolean;
}

function indentLines(code: string, prefix = '    '): string {
    return code.split('\n').map((line) => (line.trim() ? prefix + line : line)).join('\n');
}

export function emitModule(mod: IRModule, opts: EmitOptions = {}): string {
    const { includeRuntimeImport = true } = opts;
    emitOptions = opts;
    const lines: string[] = [];

    lines.push(`"/* MODULE ${mod.path} */"`);

    if (includeRuntimeImport) {
        lines.push(
/* `try:
    import js_runtime.js_runtime
except ImportError:
    import random
    class Console:
        def log(self, *args):
            print(*args)
        def dir(self, arg):
            print(dir(arg))
    console = Console()
    class Math:
        def random(self):
            random()
    math = Math()` */
`from js_runtime.js_runtime import *`
        );
    }

    for (const stmt of mod.body) {
        if (stmt.type === 'ExprStmt' && stmt.expr.type === 'Literal' && stmt.expr.value === null) continue;
        lines.push(emitNode(stmt));
    }

    return lines.join('\n\n');
}

function emitNode(node: IRNode): string {
    switch (node.type) {
        case 'Function':
            return emitFunction(node);
        case 'VarDecl':
            return emitVarDecl(node);
        case 'Assign':
            return emitAssign(node);
        case 'If':
            return emitIf(node);
        case 'Return':
            return emitReturn(node);
        case 'ExprStmt':
            return emitExprStmt(node);
        case 'Call':
            return emitCall(node);
        case 'Literal':
            return emitLiteral(node);
        case 'Identifier':
            return emitIdentifier(node);
        case 'Binary':
            return emitBinary(node);
        case 'ArrowFunction':
            return emitArrowFunction(node);
        case 'Import':
            return emitImport(node);
        case 'IIFE':
            return emitIIFE(node);
        default:
            return `# [unhandled ${node.type}]`;
    }
}

function emitFunction(fn: IRFunction): string {
    const name = fn.name ?? '_anon';
    const header = `def ${name}(${fn.params.join(', ')}):`;
    if (!fn.body || fn.body.length === 0) return header + '\n    pass';
    const body = fn.body.map(emitNode).join('\n');
    return header + '\n' + indentLines(body);
}

function emitVarDecl(decl: IRVarDecl): string {
    if (decl.init?.type === 'IIFE') {
        const iifeNode = { ...decl.init };
        const code = emitIIFE(iifeNode);
        if (!iifeNode.resultVar && iifeNode.body.length > 1) {
            const fnName = iifeNode.name ?? '_anon_iife';
            return `${code}\n${decl.name} = ${fnName}(${(iifeNode.args ?? []).map(emitNode).join(', ')})`;
        }
        return code;
    }

    if (decl.init?.type === 'ArrowFunction') {
        const arrowNode = { ...decl.init };
        const code = emitArrowFunction(arrowNode);
        if (!arrowNode.resultVar && Array.isArray(arrowNode.body)) {
            const fnName = arrowNode.name ?? `_anon_${decl.name}`;
            return `${code}\n${decl.name} = ${fnName}`;
        }
        return code;
    }

    if (decl.init?.type === 'Call') {
        const callNode = decl.init;
        const callee = callNode.callee;

        if (callee.type === 'IIFE') {
            const iifeNode = { ...callee };
            iifeNode.resultVar = undefined!;
            const defCode = emitIIFE(iifeNode);
            const callArgs = (callNode.args ?? []).map(emitNode).join(', ');
            const fnName = iifeNode.name ?? '_anon_iife';
            return `${defCode}\n${decl.name} = ${fnName}(${callArgs})`;
        }

        if (callee.type === 'ArrowFunction') {
            const arrowNode = { ...callee };
            arrowNode.resultVar = undefined!;
            const defCode = emitArrowFunction(arrowNode);
            const callArgs = (callNode.args ?? []).map(emitNode).join(', ');
            const fnName = arrowNode.name ?? `_anon_${decl.name}`;
            return `${defCode}\n${decl.name} = ${fnName}(${callArgs})`;
        }
    }

    if (decl.init) {
        return `${decl.name} = ${emitNode(decl.init)}`;
    }

    return `${decl.name} = None`;
}

function emitAssign(assign: IRAssign): string {
    return `${emitNode(assign.target)} = ${emitNode(assign.value)}`;
}

function emitIf(node: IRIf): string {
    let test = emitNode(node.test);

    if (test.startsWith('(') && test.endsWith(')')) {
        test = test.slice(1, -1);
    }
    let consLines = node.consequent.map(emitNode).filter(Boolean);
    if (consLines.length === 0) consLines = ['pass'];

    let code = `if ${test}:\n${indentLines(consLines.join('\n'))}`;

    if (node.alternate && node.alternate.length > 0) {
        let altLines = node.alternate.map(emitNode).filter(Boolean);
        if (altLines.length === 0) altLines = ['pass'];
        code += `\nelse:\n${indentLines(altLines.join('\n'))}`;
    }
    return code;
}

function emitReturn(node: IRReturn): string {
    return node.value ? `return ${emitNode(node.value)}` : 'return';
}

function emitExprStmt(node: IRExprStmt): string {
    return emitNode(node.expr);
}

function emitCall(node: IRCall): string {
    let callee = emitNode(node.callee);
    const args = node.args.map(emitNode).join(', ');

    if (!emitOptions.includeRuntimeImport) {
        if (callee === 'Array.isArray') {
            return `isInstance(${args}, list)`;
        } else if (callee === 'console.log') {
            return `print(${args})`;
        } else if (callee === 'console.dir') {
            return `print(dir(${args}))`;
        } else if (callee === 'math.random()') {
            return `random()`;
        }
    }

    return `${callee}(${args})`;
}

function emitLiteral(node: IRLiteral): string {
    if (node.value === null) return 'None';
    if (node.value === true) return 'True';
    if (node.value === false) return 'False';
    if (typeof node.value === 'string') return JSON.stringify(node.value);
    return String(node.value);
}

function emitIdentifier(node: IRIdentifier): string {
    return node.name;
}

function emitBinary(node: IRBinary): string {
    const left = emitNode(node.left);
    const right = emitNode(node.right);
    const opMap: Record<string, string> = {
        '===': '==',
        '!==': '!=',
        '&&': 'and',
        '||': 'or'
    }
    const op = opMap[node.operator] ?? node.operator;
    return `(${left} ${op} ${right})`;
}

function emitArrowFunction(node: IRArrowFunction): string {
    const paramList = (() => {
        const params = node.params.map((p, i) => {
            const def = node.defaults?.[i];
            return def ? `${p}=${emitNode(def)}` : p;
        });
        if (node.restParam) params.push(`*${node.restParam}`);
        return params.join(', ');
    })();

    const isExpr = node.isExpressionBody || node.singleReturn;
    const resultVar = node.resultVar;

    let bodyCode: string;
    if (isExpr) {
        bodyCode = Array.isArray(node.body)
            ? emitNode((node.body[0] as IRReturn).value!)
            : emitNode(node.body as IRExpr);
    } else {
        const bodyLines = (node.body as IRNode[]).map(emitNode).filter(Boolean);
        bodyCode = indentLines(bodyLines.length ? bodyLines.join('\n') : 'pass');
    }

    if (node.isIIFE) {
        const args = (node.args ?? []).map(emitNode).join(', ');

        if (isExpr) {
            return `(lambda ${paramList}: ${bodyCode})(${args})`;
        } else {
            const fnName = node.name ?? (resultVar ? `_anon_${resultVar}` : '_anon_iife');
            if (resultVar) {
                return [
                    `def ${fnName}(${paramList}):`,
                    bodyCode,
                    `${resultVar} = ${fnName}(${args})`
                ].join('\n');
            } else {
                return [
                    `def ${fnName}(${paramList}):`,
                    bodyCode,
                    `${fnName}(${args})`
                ].join('\n');
            }
        }
    }

    if (isExpr) {
        if (resultVar) return `${resultVar} = lambda ${paramList}: ${bodyCode}`;
        return `(lambda ${paramList}: ${bodyCode})`;
    }

    if (resultVar) {
        const fnName = node.name ?? `_anon_${resultVar}`;
        return [
            `def ${fnName}(${paramList}):`,
            bodyCode,
            `${resultVar} = ${fnName}`
        ].join('\n');
    }

    const fnName = node.name ?? '_anon_arrow';
    return `def ${fnName}(${paramList}):\n${bodyCode}`;
}

function emitIIFE(node: IRIIFE): string {
    const hasArgs = node.params && node.params.length > 0;
    const fnName = node.name ?? '_anon_iife';
    const callArgs = (node.args ?? []).map(emitNode).join(', ');

    const bodyLines = node.body.map(emitNode).filter(Boolean);
    const bodyCode = bodyLines.length ? bodyLines.join('\n') : 'pass';
    const indentBody = indentLines(bodyCode);

    const singleReturn = node.body.length === 1 && node.body[0]?.type === 'Return' && !hasArgs && !node.resultVar;

    if (singleReturn && !node.resultVar) {
        const returnExpr = emitNode((node.body[0] as IRReturn).value!);
        return `(lambda: ${returnExpr})()`;
    }

    if (node.resultVar) {
        return [
            `def ${fnName}(${node.params.join(', ')}):`,
            indentBody,
            `${node.resultVar} = ${fnName}(${callArgs})`
        ].join('\n');
    }

    return [
        `def ${fnName}(${node.params.join(', ')}):`,
        indentBody,
        `${fnName}(${callArgs})`
    ].join('\n');

}

function emitImport(node: IRImport): string {
    let specifier = node.specifier.replace(/^\.\/|^\.\.\//, '');
    if (specifier.endsWith('.js')) specifier = specifier.slice(0, -3);

    if (node.imported && node.imported !== 'default' && node.imported !== '*') {
        if (node.localName && node.localName !== node.imported) {
            return `from ${specifier} import ${node.imported} as ${node.localName}`;
        }
        return `from ${specifier} import ${node.imported}`;
    }

    if (node.imported === 'default' && node.localName) {
        return `import ${node.localName}  # from ${specifier}`;
    }

    if (node.imported === '*' && node.localName) {
        return `import ${node.localName}  # namespace from ${specifier}`;
    }

    return `import ${specifier}`;
}
