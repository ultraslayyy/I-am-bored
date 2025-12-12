import { execSync } from 'child_process';

const exports = [
    '_add',
    '_subtract',
    '_multiply',
    '_divide',
    '_power',
    '_sqrt',
    '_hypot_wasm',
    '_PI',
    '_E',
    '_toBase64'
];

const cmd = [
    'emcc',
    'src/wasm.cpp',
    '-O3',
    '--no-entry',
    '-s STANDALONE_WASM',
    `-s EXPORTED_FUNCTIONS="[${exports.map(x => `"${x}"`).join(',')}]"`,
    '-fvisibility=default',
    '-o wasm-build/ultra.wasm'
].join(' ');

console.log('Running:', cmd);
execSync(cmd, { stdio: 'inherit' });
