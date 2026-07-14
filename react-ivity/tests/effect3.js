import { useState, useEffect } from '../lib/js/index';

const count = useState(0);

useEffect(() => {
    if (count === 0) {
        console.log(`[Effect] Runs on start`);
    } else {
        console.log(`[Effect] This should never run`);
    }
}, []);

count.value = 1;