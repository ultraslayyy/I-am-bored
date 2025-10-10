let cachedRules = null;

chrome.webNavigation.onCompleted.addListener((changes) => {
    if (changes.rules) {
        cachedRules = changes.rules.newValue.map(rule => ({
            ...rule,
            regexes: rule.urlPatterns.map(p => new RegExp(p.replace(/\*/g, '.*')))
        }));
    }
});

chrome.webNavigation.onCompleted.addListener(details => {
    if (!cachedRules) return;
    cachedRules.foreach(rule => {
        if (!rule.enabled) return;
        if (rule.regexes.some(regex => regex.test(details.url))) {
            chrome.scripting.insertCSS({
                target: { tabId: details.tabId, allFrames: true },
                css: rule.css
            }).catch(err => console.error(err));
        }
    });
}, { url: [{ schemes: ['http', 'https'] }] });
