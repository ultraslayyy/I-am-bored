import type { IRArrowFunction, IRAssign, IRBinary, IRCall, IRExprStmt, IRFunction, IRIdentifier, IRIf, IRIIFE, IRImport, IRLiteral, IRModule, IRNode, IRReturn, IRVarDecl } from './ir.js';

interface EmitOptions {
    indent?: string;
}

let emitOptions: EmitOptions = {}

function indentLines(code: string, prefix = emitOptions.indent || '  '): string {
    return code.split('\n').map((line) => (line.trim() ? prefix + line : line)).join('\n');
}

export function emitModule(mod: IRModule, opts: EmitOptions = { indent: '  ' }): string {
    emitOptions = opts;
    const lines: string[] = [];

    lines.push(`"/* MODULE ${mod.path} */"`);

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

function emitFunction(node: IRFunction): string {
    const name = node.name ?? '_anon';
    const header = `${name} = (${node.params.join(', ')}) ->`;

    if (!node.body || node.body.length === 0) {
        return header + '\n  # pass';
    }
    
    const bodyNodes = node.body.map((n, i) => {
        const isLast = i === node.body.length - 1;
        if (isLast && n.type !== 'Return') {
            return emitNode(n);
        }
        return emitNode(n);
    });

    const body = bodyNodes.join('\n');
    return header + '\n' + indentLines(body, '  ');
}

function emitVarDecl(node: IRVarDecl): string {
    const init = node.init ? ` = ${emitNode(node.init)}` : '';
    return `${node.name}${init}`;
}

function emitAssign(node: IRAssign): string {
    return `${emitNode(node.target)} = ${emitNode(node.value)}`;
}

function emitIf(node: IRIf): string {
    let code = `if ${emitNode(node.test)}\n`;
    code += indentLines(node.consequent.map(emitNode).join('\n'));
    if (node.alternate && node.alternate.length > 0) {
        code += `\nelse\n`;
        code += indentLines(node.alternate.map(emitNode).join('\n'));
    }
    return code;
}

function emitReturn(node: IRReturn): string {
    return `return ${emitNode(node.value!)}`;
}

function emitExprStmt(node: IRExprStmt): string {
    return emitNode(node.expr);
}

function emitCall(node: IRCall): string {
    const callee = emitNode(node.callee);
    const args = node.args.map(emitNode).join(', ');
    return `${callee}(${args})`;
}

function emitLiteral(node: IRLiteral): string {
    if (typeof node.value === 'string') return JSON.stringify(node.value);
    if (node.value === null) return 'null';
    return String(node.value);
}

function emitIdentifier(node: IRIdentifier): string {
    return node.name;
}

function emitBinary(node: IRBinary): string {
    return `${emitNode(node.left)} ${node.operator} ${emitNode(node.right)}`;
}

function emitArrowFunction(node: IRArrowFunction): string {
    return '';
}

function emitImport(node: IRImport): string {
    return '';
}

function emitIIFE(node: IRIIFE): string {
    if (!node.body || node.body.length === 0) {
        return `do ->`;
    }
    const body = node.body.map(emitNode).join('\n');
    return `do ->\n` + indentLines(body);
}