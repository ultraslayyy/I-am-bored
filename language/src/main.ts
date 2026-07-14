import { Lexer } from './lexer.js';

const code = `
fn main() {
    let x = 5;
    print(x);
}
`;

const lexer = new Lexer(code);
const tokens = lexer.tokenise();

console.log(tokens);