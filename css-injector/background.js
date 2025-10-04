chrome.webNavigation.onCompleted.addListener(async details => {
    const { rules } = await chrome.storage.local.get('rules');
    if (!rules) return;

    for (let rule of rules) {
        if (!rule.enabled) continue;
        for (let pattern of rule.urlPatterns) {
            const regex = new RegExp(pattern.replace(/\*/g, '.*'));
            if (details.url.match(regex)) {
                console.log('Injecting CSS for rule:', rule.urlPatterns);
                console.log('Injecting CSS into tab:', details.tabId);
                if (details.tabId > 0) {
                    chrome.scripting.insertCSS({
                        target: { tabId: details.tabId, allFrames: true },
                        css: rule.css
                    }).catch(err => console.error('InsertCSS error:', err));
                }
                break;
            }
        }
    }
}, { url: [{ schemes: ['http', 'https'] }] });
