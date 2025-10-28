import { useState, useEffect } from '../lib/js/index.js';

const count = useState(0);

useEffect(() => {
    if (count.value === 0) {
        console.log(`[Effect] This runs on start :)`);
    } else {
        console.log(`[Effect] State value is now: ${count.value}`);
    }
});

count.value = 1;