import { join } from 'node:path';
import fs, { mkdirSync, writeFileSync } from 'node:fs';
import envPaths from 'env-paths';
import { OtpOptions, HotpOptions } from './otp';

type HotpToken = HotpOptions;
type Token = OtpOptions;

export interface TokenFile {
    tokens: Token[];
}

export const storageDir = envPaths('votp');
export const tokenFile = join(storageDir.config, 'tokens.json');

export function loadTokens(): Token[] {
    if (!fs.existsSync(tokenFile)) return [];

    const raw = fs.readFileSync(tokenFile, 'utf-8');
    const data = JSON.parse(raw) as TokenFile;
    return data.tokens || [];
}

export function findToken(label: string): Token | undefined {
    const tokens = loadTokens();

    if (tokens.find(t => t.label === label)) {
        return tokens.find(t => t.label === label);
    }
}

export function saveTokens(tokens: Token[]) {
    if (!fs.existsSync(storageDir.config)) mkdirSync(storageDir.config, { recursive: true });

    const data: TokenFile = { tokens }
    writeFileSync(tokenFile, JSON.stringify(data, null, 2), 'utf-8');
}

export function addToken(newToken: Token, force?: boolean) {
    const tokens = loadTokens();

    if (tokens.some(t => t.label === newToken.label)) {
        if (!force) {
            throw new Error(`Token with label "${newToken.label}" already exists`);
        } else {
            deleteToken(newToken.label);
            addToken(newToken);
        }
    }

    tokens.push(newToken);
    saveTokens(tokens);
}

export function bumpHotp(label: string) {
    const tokens = loadTokens();
    const token = tokens.find(t => t.label === label && t.type === 'hotp') as HotpToken;

    if (!token) throw new Error(`HOTP token "${label}" not found`);
    token.counter += 1;
    
    saveTokens(tokens);
}

export function deleteToken(label: string) {
    const tokens = loadTokens();
    const filtered = tokens.filter(t => t.label !== label);
    saveTokens(filtered);
}