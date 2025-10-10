let styleElements = {}

function applyRule(rule) {
    if (!rule.enabled) return;

    rule.urlPatterns.forEach(pattern => {
        try {
            const regex = new RegExp(pattern.replace(/\*/g, '.*'));
            if (!regex.test(window.location.href)) return;

            if (!styleElements[rule.id]) {
                const style = document.createElement('style');
                style.id = `css-rule-${rule.id}`;
                style.textContent = rule.css;
                document.head.appendChild(style);
                styleElements[rule.id] = style;
            } else {
                styleElements[rule.id].textContent = rule.css;
            }
        } catch (e) {
            console.warn('Invalid pattern:', pattern);
        }
    });
}

function removeRule(rule) {
    const style = styleElements[rule.id];
    if (style) {
        style.remove();
        delete styleElements[rule.id];
    }
}

chrome.storage.onChanged.addListener((changes) => {
    if (changes.rules) {
        const newRules = changes.rules.newValue || [];
        const oldRules = changes.rules.oldValue || [];

        oldRules.forEach(oldRule => {
            if (!newRules.find(r => r.id === oldRule.id)) removeRule(oldRule);
        });

        newRules.forEach(rule => applyRule(rule));
    }
});

chrome.storage.local.get('rules', ({ rules }) => {
    if (!rules) return;
    rules.forEach(rule => applyRule(rule));
});
