import { type Token, TokenType } from './token.js';

const KEYWORDS: Record<string, TokenType> = {
    let: TokenType.Let,
    mut: TokenType.Mut,
    fn: TokenType.Fn,
    return: TokenType.Return,
    if: TokenType.If,
    else: TokenType.Else,
    while: TokenType.While,
    struct: TokenType.Struct,
    true: TokenType.True,
    false: TokenType.False
}

export class Lexer {
    #pos = 0;
    #input = '';
    #tokens: Token[] = [];

    constructor(input: string) {
        this.#input = input;
    }

    tokenise(): Token[] {
        while (!this.#isAtEnd()) {
            const c = this.#peek();

            if (/\s/.test(c)) {
                this.#advance();
                continue;
            }

            if (/[0-9]/.test(c)) {
                this.#tokens.push(this.#number());
                continue;
            }

            if (/[a-zA-Z_]/.test(c)) {
                this.#tokens.push(this.#identifier());
                continue;
            }

            if (c === '"') {
                this.#tokens.push(this.#string());
                continue;
            }

            switch (c) {
                case '+':
                    this.#tokens.push({ type: TokenType.Plus });
                    break;
                case '-':
                    if (this.#peekNext() === '>') {
                        this.#advance();
                        this.#advance();
                        this.#tokens.push({ type: TokenType.Arrow });
                        continue;
                    }
                    this.#tokens.push({ type: TokenType.Minus });
                    break;
                case '*':
                    this.#tokens.push({ type: TokenType.Star });
                    break;
                case '/':
                    this.#tokens.push({ type: TokenType.Slash });
                    break;
                case '%':
                    this.#tokens.push({ type: TokenType.Percent });
                    break;
                case '=':
                    if (this.#peekNext() === '=') {
                        this.#advance();
                        this.#advance();
                        this.#tokens.push({ type: TokenType.EqualsEquals });
                        continue;
                    }
                    this.#tokens.push({ type: TokenType.Equals });
                    break;
                case '!':
                    if (this.#peekNext() === '=') {
                        this.#advance();
                        this.#advance();
                        this.#tokens.push({ type: TokenType.BangEquals });
                        continue;
                    }
                    throw new Error('Unexpected \'!\'');
                case '<':
                    if (this.#peekNext() === '=') {
                        this.#advance();
                        this.#advance();
                        this.#tokens.push({ type: TokenType.LessEquals });
                        continue;
                    }
                    this.#tokens.push({ type: TokenType.Less });
                    break;
                case '>':
                    if (this.#peekNext() === '=') {
                        this.#advance();
                        this.#advance();
                        this.#tokens.push({ type: TokenType.GreaterEquals });
                        continue;
                    }
                    this.#tokens.push({ type: TokenType.Greater });
                    break;
                case '(':
                    this.#tokens.push({ type: TokenType.LParen });
                    break;
                case ')':
                    this.#tokens.push({ type: TokenType.RParen });
                    break;
                case '{':
                    this.#tokens.push({ type: TokenType.LBrace });
                    break;
                case '}':
                    this.#tokens.push({ type: TokenType.RBrace });
                    break;
                case ';':
                    this.#tokens.push({ type: TokenType.Semicolon });
                    break;
                case ':':
                    this.#tokens.push({ type: TokenType.Colon });
                    break;
                case ',':
                    this.#tokens.push({ type: TokenType.Comma });
                    break;
                default:
                    throw new Error(`Unexpected character: ${c}`);
            }

            this.#advance();
        }

        this.#tokens.push({ type: TokenType.EOF });
        return this.#tokens;
    }

    #number(): Token {
        let start = this.#pos;

        while (/[0-9]/.test(this.#peek())) this.#advance();
        
        if (this.#peek() === '.' && /[0-9]/.test(this.#peekNext())) {
            this.#advance();
            while (/[0-9]/.test(this.#peek())) this.#advance();
        }

        const value = this.#input.slice(start, this.#pos);
        return { type: TokenType.Number, value }
    }

    #identifier(): Token {
        let start = this.#pos;

        while (/[a-zA-Z_]/.test(this.#peek())) this.#advance();

        const text = this.#input.slice(start, this.#pos);

        const keyword = KEYWORDS[text];
        if (keyword !== undefined) {
            return { type: keyword }
        }

        return { type: TokenType.Identifier, value: text }
    }

    #string(): Token {
        this.#advance();

        let start = this.#pos;

        while (this.#peek() !== '"' && !this.#isAtEnd()) {
            this.#advance();
        }

        const value = this.#input.slice(start, this.#pos);
        this.#advance();

        return { type: TokenType.String, value }
    }

    #peek(): string {
        return this.#input[this.#pos] ?? '\0';
    }

    #peekNext(): string {
        return this.#input[this.#pos + 1] ?? '\0';
    }

    #advance() {
        this.#pos++;
    }

    #isAtEnd(): boolean {
        return this.#pos >= this.#input.length;
    }
}