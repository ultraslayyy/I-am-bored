export type Program = {
    type: 'Program';
    body: Statement[];
}

export type Statement =
    | VariableDeclaration
    | FunctionDeclaration
    | ExpressionStatement
    | IfStatement
    | WhileStatement
    | ReturnStatement;

export type Expression =
    | BinaryExpression
    | Literal
    | Identifier
    | CallExpression;

export type VariableDeclaration = {
    type: 'VariableDeclaration';
    name: string;
    mutable: boolean;
    varType?: string | undefined;
    value: Expression;
}

export type FunctionDeclaration = {
    type: 'FunctionDeclaration';
    name: string;
    params: { name: string, type: string }[];
    returnType?: string;
    body: Statement[];
}

export type ExpressionStatement = {
    type: 'ExpressionStatement';
    expression: Expression;
}

export type IfStatement = {
    type: 'IfStatement';
    condition: Expression;
    thenBranch: Statement[];
    elseBranch?: Statement[];
}

export type WhileStatement = {
    type: 'WhileStatement';
    condition: 'Expression';
    body: Statement[];
}

export type ReturnStatement = {
    type: 'ReturnStatement';
    value?: Expression;
}

export type BinaryExpression = {
    type: 'Binary Expression';
    operator: string;
    left: Expression;
    right: Expression;
}

export type Literal = {
    type: 'Literal';
    value: string;
}

export type Identifier = {
    type: 'Identifier';
    name: string;
}

export type CallExpression = {
    type: 'CallExpression';
    callee: string;
    args: Expression[];
}