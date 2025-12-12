try {
    const native = require('./build/Release/addon.node');
    module.exports = native;
    return;
} catch (e) {
    console.warn('Native addon not loaded, falling back to WASM.');
}

async function initWasm() {
    const isNode = (typeof window === 'undefined');
    const fs = isNode ? require('fs') : null;

    let wasmBinary;

    if (fs) {
        wasmBinary = fs.readFileSync(__dirname + '/wasm-build/ultra.wasm');
    } else {
        const res = await fetch('/ultra.wasm');
        wasmBinary = await res.arrayBuffer();
    }

    const { instance } = await WebAssembly.instantiate(wasmBinary, {
        env: {
            abort: () => console.error('WASM Abort')
        }
    });

    const wasmExports = instance.exports;

    return {
        add: wasmExports._add,
        subtract: wasmExports._subtract,
        multiply: wasmExports._multiply,
        divide: wasmExports._divide,
        power: wasmExports._power,
        sqrt: wasmExports._sqrt,
        hypot: wasmExports._hypot_wasm,
        PI: wasmExports._PI,
        E: wasmExports._E,
        toBase64: wasmExports._toBase64
    }
}

module.exports = initWasm();