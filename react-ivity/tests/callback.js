import { useState, useCallback } from '../lib/js/index.js';

const count = useState(0);
const multiplier = useState(2);

function logicBlock() {
    const memoizedFunction = useCallback(() => {
        const result = count.value * multiplier.value;
        console.log(`[Callback Executed] Result: ${result}`);
        return result;
    }, [count.value, multiplier.value]);

    console.log(`[logicBlock] Function created or retrieved from cache.`);

    return memoizedFunction;

}

// Should create
const fn1 = logicBlock();
fn1();

// Should return cached function
const fn2 = logicBlock();
console.log('Function reference is the same:', fn1 === fn2); // false
fn2();

count.value = 5;

const fn3 = logicBlock();
console.log('Function reference is the same:', fn1 === fn3); // false
fn3();