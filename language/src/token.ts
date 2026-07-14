export enum TokenType {
    // Literals
    Number,
    String,
    Identifier,

    // Keywords
    Let,
    Mut,
    Fn,
    Return,
    If,
    Else,
    While,
    Struct,
    True,
    False,

    // Operators
    Plus,
    Minus,
    Star,
    Slash,
    Percent,
    Equals,
    EqualsEquals,
    BangEquals,
    Less,
    LessEquals,
    Greater,
    GreaterEquals,

    // Symbols
    Colon,
    Semicolon,
    Comma,
    LParen,
    RParen,
    LBrace,
    RBrace,
    Arrow, // ->

    EOF
}

export type Token = {
    type: TokenType,
    value?: string;
}