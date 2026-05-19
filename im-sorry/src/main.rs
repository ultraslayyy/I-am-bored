use std::io;

enum Token {
    Number(f64),
    Op(char)
}

fn tokenise(input: &str) -> Vec<Token> {
    let mut tokens = Vec::new();
    let mut num = String::new();

    for c in input.chars() {
        if c.is_digit(10) || c == '.' {
            num.push(c);
        } else if "+-*/".contains(c) {
            if !num.is_empty() {
                tokens.push(Token::Number(num.parse().unwrap()));
                num.clear();
            }
            tokens.push(Token::Op(c));
        }
    }

    if !num.is_empty() {
        tokens.push(Token::Number(num.parse().unwrap()));
    }

    tokens
}

fn main() {
    let mut input = String::new();
    io::stdin().read_line(&mut input).unwrap();

    let tokens = tokenise(&input);

    let mut iter = tokens.into_iter();
    let mut result = match iter.next().unwrap() {
        Token::Number(n) => n,
        _ => panic!("Expected number")
    };

    while let Some(token) = iter.next() {
        let op = match token {
            Token::Op(c) => c,
            _ => panic!("Expected operator")
        };

        let next_num = match iter.next().unwrap() {
            Token::Number(n) => n,
            _ => panic!("Expected number")
        };

        result = match op {
            '+' => result + next_num,
            '-' => result - next_num,
            '*' => result * next_num,
            '/' => result / next_num,
            _ => panic!("Unknown operator"),
        };
    }

    println!("{}", result);
}
