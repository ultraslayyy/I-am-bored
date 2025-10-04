import type { File, Program } from '@babel/types';
import * as t from '@babel/types';
import type { IRModule, IRNode, IRExpr, IRFunction, IRVarDecl, IRReturn, IRExprStmt, IRLiteral, IRIdentifier, IRCall, IRIf, IRIIFE, IRImport, IRBinary, IRArrowFunction } from './ir.js';

export function astToIR(ast: File, path = '<input>'): IRModule {
    const body: IRNode[] = [];
    const program: Program = ast.program;

    for (const node of program.body) {
        const ir = stmtToIR(node);
        if (Array.isArray(ir)) body.push(...ir);
        else if (ir) body.push(ir);
    }

    return { type: 'Module', path, body }
}

function stmtToIR(node: t.Node, parent?: t.Node): IRNode | IRNode[] | null {
    if (t.isFunctionDeclaration(node)) {
        const name = node.id ? node.id.name : '_anon';
        const params = node.params.map(p => (t.isIdentifier(p) ? p.name : '_arg'));
        const bodyNodes: IRNode[] = [];
        for (const st of node.body.body) {
            const ir = stmtToIR(st, node);
            if (Array.isArray(ir)) bodyNodes.push(...ir);
            else if (ir) bodyNodes.push(ir);
        }
        const fn: IRFunction = { type: 'Function', name, params, body: bodyNodes }
        return fn;
    }

    if (t.isVariableDeclaration(node)) {
        const outs: IRNode[] = [];
        for (const decl of node.declarations) {
            if (t.isIdentifier(decl.id)) {
                const name = decl.id.name;
                const init = decl.init ? exprToIR(decl.init, decl) : null;
                outs.push({ type: 'VarDecl', name, init } as IRVarDecl);
            }
        }
        return outs;
    }

    if (t.isExpressionStatement(node)) {
        const e = exprToIR(node.expression);
        if (e) return { type: 'ExprStmt', expr: e } as IRExprStmt;
        return null;
    }

    if (t.isReturnStatement(node)) {
        const val = node.argument ? exprToIR(node.argument) : null;
        return { type: 'Return', value: val } as IRReturn;
    }

    if (t.isIfStatement(node)) {
        const test = exprToIR(node.test) as IRBinary | IRIdentifier | IRLiteral;
        const cons: IRNode[] = [];
        const alt: IRNode[] = [];

        if (t.isBlockStatement(node.consequent)) {
            for (const st of node.consequent.body) {
                const ir = stmtToIR(st, node);
                if (Array.isArray(ir)) cons.push(...ir);
                else if (ir) cons.push(ir);
            }
        } else {
            const ir = stmtToIR(node.consequent, node);
            if (Array.isArray(ir)) cons.push(...ir);
            else if (ir) cons.push(ir);
        }

        if (node.alternate) {
            if (t.isBlockStatement(node.alternate)) {
                for (const st of node.alternate.body) {
                    const ir = stmtToIR(st, node);
                    if (Array.isArray(ir)) alt.push(...ir);
                    else if (ir) alt.push(ir);
                }
            } else {
                const ir = stmtToIR(node.alternate, node);
                if (Array.isArray(ir)) alt.push(...ir);
                else if (ir) alt.push(ir);
            }
        }
        
        return { type: 'If', test, consequent: cons, alternate: alt.length ? alt : null } as IRIf;
    }

    if (t.isImportDeclaration(node)) {
        const spec = node.source.value as string;
        const ins: IRNode[] = [];
        for (const s of node.specifiers) {
            let localName: string = '';
            let imported: string = '';
            if (t.isImportSpecifier(s)) {
                imported = (s.imported as t.Identifier).name;
                localName = (s.local as t.Identifier).name;
            } else if (t.isImportDefaultSpecifier(s)) {
                imported = 'default';
                localName = (s.local as t.Identifier).name;
            } else if (t.isImportNamespaceSpecifier(s)) {
                imported = '*';
                localName = (s.local as t.Identifier).name;
            }
            ins.push({ type: 'Import', specifier: spec, imported, localName} as IRImport);
        }
        return ins;
    }
    
    if (t.isExportNamedDeclaration(node) && node.declaration) {
        return stmtToIR(node.declaration);
    }

    return { type: 'ExprStmt', expr: { type: 'Literal', value: null } } as IRExprStmt;
}

function exprToIR(node: t.Expression | t.Pattern | null, parent?: t.Node): any {
    if (!node) return null;
    if (t.isIdentifier(node)) return { type: 'Identifier', name: node.name } as IRIdentifier;
    if (t.isNumericLiteral(node) || t.isStringLiteral(node) || t.isBooleanLiteral(node) || t.isNullLiteral(node)) {
        return { type: 'Literal', value: (node as any).value } as IRLiteral;
    }
    if (t.isCallExpression(node)) {
        if (t.isFunctionExpression(node.callee)) {
            const fnNode = node.callee;
            const params = fnNode.params.map(p => (t.isIdentifier(p) ? p.name : '_arg'));

            let bodyNodes: IRNode[] = [];
            if (t.isBlockStatement(fnNode.body)) {
                for (const st of fnNode.body.body) {
                    const ir = stmtToIR(st, node);
                    if (Array.isArray(ir)) bodyNodes.push(...ir);
                    else if (ir) bodyNodes.push(ir);
                }
            } else {
                const expr = exprToIR(fnNode.body);
                if (expr) bodyNodes.push({ type: 'ExprStmt', expr } as IRExprStmt);
            }

            const args = node.arguments
                .filter((arg): arg is t.Expression => t.isExpression(arg))
                .map(a => exprToIR(a, node));

            let resultVar: string | null = null;
            if (parent && t.isVariableDeclarator(parent) && t.isIdentifier(parent.id)) {
                resultVar = parent.id.name;
            }

            return {
                type: 'IIFE',
                params,
                body: bodyNodes,
                name: fnNode.id?.name ?? null,
                args,
                resultVar
            } as IRIIFE;
        }
        return { type: 'Call', callee: exprToIR(node.callee as t.Expression, node), args: node.arguments.map(a => exprToIR(a as t.Expression, node)) } as IRCall;
    }
    if (t.isBinaryExpression(node) || t.isLogicalExpression(node)) {
        return { type: 'Binary', operator: node.operator, left: exprToIR(node.left as t.Expression), right: exprToIR(node.right) } as IRBinary;
    }
    if (t.isArrowFunctionExpression(node)) {
        const { names: params, defaults, restParam } = extractParams(node.params, node);
        const isExpressionBody = !t.isBlockStatement(node.body);
        let body: IRNode[] | IRExpr;
        let singleReturn = false;

        if (t.isBlockStatement(node.body)) {
            const bodyNodes: IRNode[] = [];
            for (const st of node.body.body) {
                const ir = stmtToIR(st, node);
                if (Array.isArray(ir)) bodyNodes.push(...ir);
                else if (ir) bodyNodes.push(ir);
            }
            body = bodyNodes;

            if (bodyNodes.length === 1 && bodyNodes[0]?.type === 'Return') {
                const ret = bodyNodes[0] as IRReturn;
                if (ret.value) singleReturn = true;
            }
        } else {
            body = exprToIR(node.body, node);
            singleReturn = true;
        }

        let resultVar: string | null = null;
        if (parent && t.isVariableDeclarator(parent) && t.isIdentifier(parent.id)) {
            resultVar = parent.id.name;
        } else if (parent && t.isAssignmentExpression(parent) && t.isIdentifier(parent.left)) {
            resultVar = parent.left.name;
        }

        let isIIFE = false;
        let args: IRExpr[] | undefined;
        if (parent && t.isCallExpression(parent) && parent.callee === node) {
            isIIFE = true;
            args = parent.arguments.filter((arg): arg is t.Expression => t.isExpression(arg)).map(arg => exprToIR(arg, parent));
        }

        const usedAsExpr = isUsedAsExpression(parent, node);
        const name = (node as any).id?.name ?? undefined;

        return {
            type: 'ArrowFunction',
            params,
            defaults,
            restParam: restParam ?? null,
            body,
            isExpressionBody,
            singleReturn,
            isIIFE,
            args,
            resultVar,
            name,
            usedAsExpr
        } as IRArrowFunction;
    }
    if (t.isMemberExpression(node)) {
        const objIR = exprToIR(node.object);
        let objName: string;
        if (objIR.type === 'Identifier') {
            objName = objIR.name;
        } else if (objIR.type === 'Literal') {
            objName = JSON.stringify(objIR.value);
        } else {
            objName = '_unknown';
        }

        const prop = node.computed
            ? `[${exprToIR(node.property as t.Expression).value ?? '_unknown'}]`
            : `.${(node.property as t.Identifier).name}`;

        return { type: 'Identifier', name: `${objName}${prop}` } as IRIdentifier;
    }
    if (t.isUnaryExpression(node)) {
        const arg = exprToIR(node.argument);
        return { type: 'Call', callee: { type: 'Identifier', name: node.operator }, args: [arg] } as IRCall;
    }

    return { type: 'Literal', value: null } as IRLiteral;
}

function extractParams(paramNodes: (t.Identifier | t.AssignmentPattern | t.RestElement | t.Pattern)[], parentForDefaults?: t.Node) {
    const names: string[] = [];
    const defaults: (any | null)[] = []; // use IRExpr once you call exprToIR
    let restParam: string | null = null;

    for (let i = 0; i < paramNodes.length; i++) {
        const p = paramNodes[i] as any;
        if (t.isIdentifier(p)) {
            names.push(p.name);
            defaults.push(null);
        } else if (t.isAssignmentPattern(p)) {
            // param like `a = 5`
            const left = p.left;
            if (t.isIdentifier(left)) names.push(left.name);
            else names.push('_arg');

            // record default as IR (use exprToIR to convert)
            defaults.push(exprToIR(p.right as t.Expression, parentForDefaults));
        } else if (t.isRestElement(p)) {
            // rest param like ...rest
            const arg = p.argument;
            if (t.isIdentifier(arg)) {
                restParam = arg.name;
            } else {
                restParam = '_rest';
            }
            // rest param does not add to `names`/`defaults` for normal positional params
        } else {
            // destructuring (object/array) - fallback to placeholder param
            names.push('_arg');
            defaults.push(null);
        }
    }

    return { names, defaults, restParam };
}

function isUsedAsExpression(parent?: t.Node, node?: t.Node): boolean {
    if (!parent) return false;

    // If it's an IIFE (parent is CallExpression and callee === node) — not "used as expr" in callback sense
    if (t.isCallExpression(parent) && parent.callee === node) return false;

    // If assigned to var (VariableDeclarator init) or top-level ExpressionStatement directly, treat as not used-as-expr
    if (t.isVariableDeclarator(parent) && parent.init === node) return false;
    if (t.isExpressionStatement(parent) && parent.expression === node) return false;
    if (t.isExportNamedDeclaration(parent) && parent.declaration === node) return false;

    // Common expression contexts where arrow should be emitted as inline lambda:
    if (t.isCallExpression(parent)) return true;            // passed as argument
    if (t.isMemberExpression(parent)) return true;          // part of member expression
    if (t.isBinaryExpression(parent) || t.isLogicalExpression(parent)) return true;
    if (t.isConditionalExpression(parent)) return true;     // ternary: a ? (()=>...) : ...
    if (t.isReturnStatement(parent)) return true;           // returned inline
    if (t.isArrayExpression(parent)) return true;           // [() => ...]
    if (t.isObjectProperty(parent) || t.isProperty(parent)) return true; // { k: () => ... }
    if (t.isAssignmentExpression(parent) && parent.right === node) return true;
    if (t.isNewExpression(parent)) return true;
    if (t.isAwaitExpression(parent)) return true;
    if (t.isUnaryExpression(parent)) return true;
    if (t.isSequenceExpression(parent)) return true;
    if (t.isJSXExpressionContainer(parent) && (parent as any).type === 'JSXExpressionContainer') return true;

    // default: not used as inline expression
    return false;
}
