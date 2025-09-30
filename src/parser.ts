import { parse as babelParse } from '@babel/parser';
import { type File } from '@babel/types';
import fs from 'node:fs';

export function parseFile(path: string): File {
    const src = fs.readFileSync(path, 'utf-8');
    return parseSource(src, path);
}

export function parseSource(src: string, filename = '<input>'): File {
    return babelParse(src, {
        sourceType: 'module',
        plugins: [
            'jsx',
            'classProperties',
            'optionalChaining',
            'nullishCoalescingOperator',
            'objectRestSpread',
            'dynamicImport',
            'decorators-legacy',
            'topLevelAwait'
        ],
        sourceFilename: filename
    });
}