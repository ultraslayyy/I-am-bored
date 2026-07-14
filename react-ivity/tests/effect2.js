import { useState, useEffect } from '../lib/js/index.js';

const count = useState(0);
const count2 = useState(1);

useEffect(() => {
    console.log(`[Effect] Count is now: ${count.value}`);
}, [count.value, count2.value]);

count.value = 1;
count2.value = 2;