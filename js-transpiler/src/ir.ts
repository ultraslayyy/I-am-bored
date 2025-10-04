import * as t from '@babel/types';
export type IRNode = IRModule | IRFunction | IRVarDecl | IRAssign | IRIf | IRReturn | IRExprStmt | IRCall | IRLiteral | IRIdentifier | IRImport | IRBinary | IRArrowFunction | IRIIFE;

export interface IRModule {
    type: 'Module';
    path: string;
    body: IRNode[];
}

export interface IRClass {
    type: 'Class';
    name: string;
}

export interface IRFunction {
    type: 'Function';
    name: string | null;
    params: string[];
    body: IRNode[];
}

export interface IRIIFE {
    type: 'IIFE';
    name: string | null;
    params: string[];
    args: IRNode[];
    body: IRNode[];
    resultVar?: string;
}

export interface IRVarDecl {
    type: 'VarDecl';
    name: string;
    init: IRExpr | null;
}

export interface IRAssign {
    type: 'Assign';
    target: IRIdentifier;
    value: IRExpr;
}

export interface IRIf {
    type: 'If';
    test: IRExpr;
    consequent: IRNode[];
    alternate: IRNode[] | null;
}

export interface IRReturn {
    type: 'Return';
    value: IRExpr | null;
}

export interface IRExprStmt {
    type: 'ExprStmt';
    expr: IRExpr;
}

export type IRExpr = IRCall | IRLiteral | IRIdentifier | IRBinary | IRArrowFunction | IRIIFE;

export interface IRCall {
    type: 'Call';
    callee: IRExpr;
    args: IRExpr[];
}

export interface IRLiteral {
    type: 'Literal';
    value: any;
}

export interface IRIdentifier {
    type: 'Identifier';
    name: string;
}

export interface IRBinary {
    type: 'Binary';
    operator: string;
    left: IRExpr;
    right: IRExpr;
}

export interface IRArrowFunction {
    type: 'ArrowFunction';
    params: string[];
    defaults?: (IRExpr | null)[];
    restParam?: string | null;
    body: IRExpr | IRNode[];
    isExpressionBody?: boolean;
    resultVar?: string;
    args?: IRExpr[];
    name?: string;
    isIIFE?: boolean;
    singleReturn?: boolean;
    usedAsExpr?: boolean;
    _parent?: t.Node;
}

export interface IRImport {
    type: 'Import';
    specifier: string;
    imported: string | null;
    localName: string | null;
}