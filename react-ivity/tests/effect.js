import { useState, useEffect } from '../lib/js/index.js';

const count = useState(0);

useEffect(() => {
    console.log(`[Effect] State value is now: ${count.value}`);
});

count.value = 1;