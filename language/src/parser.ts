import { TokenType, type Token } from './token.js';
import * as AST from './ast.js'

export class Parser {
    #pos = 0;
    #tokens: Token[] = [];

    constructor(tokens: Token[]) {
        this.#tokens = tokens;
    }

    parse(): AST.Program {
        const body: AST.Statement[] = [];

        while (!this.#isAtEnd()) {
            body.push(this.#statement());
        }

        return { type: 'Program', body }
    }

    // @ts-ignore
    #statement(): AST.Statement {
        if (this.#match(TokenType.Let)) return this.#VariableDeclaration();
    }

    #VariableDeclaration(): AST.VariableDeclaration {
        const mutable = this.#match(TokenType.Mut);
        const name = this.#consume(TokenType.Identifier, 'Expected variable name').value!;
        let varType: string | undefined;

        if (this.#match(TokenType.Colon)) {
            varType = this.#consume(TokenType.Identifier, "Expected type").value!;
        }

        this.#consume(TokenType.Equals, 'Expected \'=\'');

        const value = this.#expression();

        this.#consume(TokenType.Semicolon, 'Expected \':\'');

        return {
            type: 'VariableDeclaration',
            name,
            mutable,
            varType,
            value
        }
    }

    #expression(): AST.Expression {
        return this.#equality();
    }

    #equality(): AST.Expression {
        let expr = this.#comparison();

        while (this.#match(TokenType.Equals, TokenType.BangEquals)) {
            const operator = this.#previous().type;
            const right = this.#comparison();

            expr = {
                type: 'Binary Expression',
                operator: TokenType[operator],
                left: expr,
                right
            }
        }

        return expr;
    }

    #comparison(): AST.Expression {
        let expr = this.#term();

        while (this.#match(
            TokenType.Less,
            TokenType.LessEquals,
            TokenType.Greater,
            TokenType.GreaterEquals
        )) {
            const operator = this.#previous().type;
            const right = this.#term();

            expr = {
                type: 'Binary Expression',
                operator: TokenType[operator],
                left: expr,
                right
            }
        }

        return expr;
    }

    #term(): AST.Expression {
        let expr = this.#factor();

        while (this.#match(TokenType.Plus, TokenType.Minus)) {
            const operator = this.#previous().type;
            const right = this.#factor();

            expr = {
                type: 'Binary Expression',
                operator: TokenType[operator],
                left: expr,
                right
            }
        }

        return expr;
    }

    #factor(): AST.Expression {
        let expr = this.#unary();

        while (this.#match(TokenType.Star, TokenType.Slash, TokenType.Percent)) {
            const operator = this.#previous().type;
            const right = this.#unary();

            expr = {
                type: 'Binary Expression',
                operator: TokenType[operator],
                left: expr,
                right
            }
        }

        return expr;
    }

    #unary(): AST.Expression {
        return this.#primary();
    }

    #primary(): AST.Expression {
        if (this.#match(TokenType.Number, TokenType.String)) {
            return { type: 'Literal', value: this.#previous().value! }
        }

        if (this.#match(TokenType.Identifier)) {
            const name = this.#previous().value!;

            if (this.#match(TokenType.LParen)) {
                const args: AST.Expression[] = [];

                if (this.#check(TokenType.RParen)) {
                    do {
                        args.push(this.#expression());
                    } while (this.#match(TokenType.Comma));
                }

                this.#consume(TokenType.RParen, 'Expected \')\'');

                return { type: 'CallExpression', callee: name, args }
            }

            return { type: 'Identifier', name }
        }

        if (this.#match(TokenType.LParen)) {
            const expr = this.#expression();
            this.#consume(TokenType.RParen, 'Expected \')\'');
            return expr;
        }

        throw new Error('Unexpected token');
    }

    #match(...types: TokenType[]): boolean {
        for (const type of types) {
            if (this.#check(type)) {
                this.#advance();
                return true;
            }
        }
        return false;
    }

    #check(type: TokenType): boolean {
        if (this.#isAtEnd()) return false;
        return this.#peek().type === type;
    }

    #advance(): Token {
        if (!this.#isAtEnd()) this.#pos++;
        return this.#previous();
    }

    #peek(): Token {
        return this.#tokens[this.#pos]!;
    }

    #previous(): Token {
        return this.#tokens[this.#pos - 1]!;
    }

    #isAtEnd(): boolean {
        return this.#peek().type === TokenType.EOF;
    }

    #consume(type: TokenType, msg: string): Token {
        if (this.#check(type)) return this.#advance();
        throw new Error(msg);
    }
}