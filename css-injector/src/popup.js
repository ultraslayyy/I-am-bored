const cssInput = document.getElementById('cssInput');
const fileInput = document.getElementById('fileInput');
const livePreview = document.getElementById('livePreview');
let previewStyleId = 'custom-css-preview-style';
const rulesList = document.getElementById('rulesList');
const patternsContainer = document.getElementById('patternsContainer');
const addPatternBtn = document.getElementById('addPatternBtn');
const addBtn = document.getElementById('addBtn');
const exportBtn = document.getElementById('exportBtn');
const importInput = document.getElementById('importFile');

addPatternBtn.addEventListener('click', () => {
    const input = document.createElement('input');
    input.type = 'text';
    input.className = 'pattern-input';
    input.placeholder = 'Site pattern (e.g. * or https://*.example.com/*)';
    const removeBtn = document.createElement('button');
    removeBtn.textContent = 'x';
    removeBtn.className = 'remove-pattern';
    removeBtn.type = "button";
    removeBtn.addEventListener('click', () => {
        input.remove();
        removeBtn.remove();
    });
    patternsContainer.appendChild(input);
    patternsContainer.appendChild(removeBtn);
});

addBtn.addEventListener('click', async () => {
    let css = cssInput.value.trim();
    if (fileInput.files.length > 0) {
        css = await fileInput.files[0].text();
    }
    if (!css) return alert('Please provide CSS');

    const urlPatterns = Array.from(document.querySelectorAll('.pattern-input'))
        .map(i => i.value.trim())
        .filter(s => s.length > 0);
    if (urlPatterns.length === 0) urlPatterns.push('*');

    let { rules } = await chrome.storage.local.get('rules');
    if (!rules) rules = [];

    rules.push({
        id: Date.now(),
        urlPatterns,
        css,
        enabled: true
    });

    await chrome.storage.local.set({ rules });
    renderRules(rules);

    cssInput.value = '';
    fileInput.value = '';
    document.querySelectorAll('.pattern-input').forEach((input, idx) => {
        if (idx === 0) input.value = '';
        else input.remove();
    });
    document.querySelectorAll('.remove-pattern').forEach(btn => btn.remove());
});

async function renderRules(rules) {
    rulesList.innerHTML = '';
    rules.forEach(rule => {
        const li = document.createElement('li');
        li.className = 'rule';
        li.innerHTML = `
            <label>
              <input type="checkbox" class="enableToggle" data-id="${rule.id}" ${rule.enabled ? 'checked' : ''}>
              Enabled
            </label>
            <b>Patterns:</b> ${rule.urlPatterns.join(', ')}<br>
            <pre>${rule.css.substring(0, 100)}${rule.css.length > 100 ? '...' : ''}</pre>
            <button data-id='${rule.id}' class='editBtn'>Edit</button>
            <button data-id='${rule.id}' class='deleteBtn'>Delete</button>
        `;
        rulesList.appendChild(li);
    });

    document.querySelectorAll('.enableToggle').forEach(cb => {
        cb.addEventListener('change', async e => {
            const id = Number(e.target.dataset.id);
            let { rules } = await chrome.storage.local.get('rules');
            rules = rules.map(r => r.id === id ? { ...r, enabled: e.target.checked } : r);
            await chrome.storage.local.set({ rules });
        });
    });

    document.querySelectorAll('.deleteBtn').forEach(btn => {
        btn.addEventListener('click', async e => {
            const id = Number(e.target.dataset.id);
            let { rules } = await chrome.storage.local.get('rules');
            rules = rules.filter(r => r.id !== id);
            await chrome.storage.local.set({ rules });
            renderRules(rules);
        });
    });

    document.querySelectorAll('.editBtn').forEach(btn => {
        btn.addEventListener('click', async e => {
            const id = Number(e.target.dataset.id);
            let { rules } = await chrome.storage.local.get('rules');
            const rule = rules.find(r => r.id === id);
            if (!rule) return;

            document.querySelectorAll('.pattern-input').forEach((input, idx) => {
                if (idx === 0) input.value = '';
                else input.remove();
            });
            document.querySelectorAll('.remove-pattern').forEach(btn => btn.remove());

            rule.urlPatterns.forEach((pattern, idx) => {
                if (idx === 0) patternsContainer.querySelector('.pattern-input').value = pattern;
                else {
                    const input = document.createElement('input');
                    input.type = 'text';
                    input.className = 'pattern-input';
                    input.value = pattern;
                    input.placeholder = 'Site pattern (e.g. * or https://*.example.com/*)';
                    const removeBtn = document.createElement('button');
                    removeBtn.textContent = 'x';
                    removeBtn.className = 'remove-pattern';
                    removeBtn.addEventListener('click', () => { input.remove(); removeBtn.remove(); });
                    patternsContainer.appendChild(input);
                    patternsContainer.appendChild(removeBtn);
                }
            });

            cssInput.value = rule.css;
            fileInput.value = '';

            rules = rules.filter(r => r.id !== id);
            await chrome.storage.local.set({ rules });
            renderRules(rules);
        });
    });
}

exportBtn.addEventListener('click', async e => {
    let { rules } = await chrome.storage.local.get('rules');
    if (!rules) rules = [];

    const json = JSON.stringify(rules, null, 2);

    const blob = new Blob([json], { type: 'application/json' });
    const url = URL.createObjectURL(blob);

    const a = document.createElement('a');
    a.href = url;
    a.download = 'custom-css-rules.json';
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);

    URL.revokeObjectURL(url);
});

importInput.addEventListener('change', async e => {
    const file = e.target.files[0];
    if (!file) return;

    const reader = new FileReader();

    reader.onload = async e => {
        try {
            const importedRules = JSON.parse(e.target.result);
            if (!Array.isArray(importedRules)) {
                alert('Invalid JSON format. Expected an array of rules.');
                return;
            }

            let { rules: existingRules } = await chrome.storage.local.get('rules');
            if (!existingRules) existingRules = [];

            const rules = [...existingRules];
            importedRules.forEach(rule => {
                if (!rules.some(r => r.urlPatterns.join(',') === rule.urlPatterns.join(',') && r.css === rule.css)) {
                    rules.push(rule);
                }
            });
            await chrome.storage.local.set({ rules });

            alert('Import successful');
            location.reload();
        } catch (err) {
            alert(`Error importing file: ${err.message}`);
        }
    }

    reader.readAsText(file);
});

let liveTimeout;
cssInput.addEventListener('input', () => {
    if (livePreview.checked) {
        clearTimeout(liveTimeout);
        liveTimeout = setTimeout(() => applyLivePreview(cssInput.value), 200);
    }
});

livePreview.addEventListener('change', () => {
    if (livePreview.checked) {
        applyLivePreview(cssInput.value);
    }  else {
        removeLivePreview();
    }
});

function applyLivePreview(css) {
    chrome.tabs.query({ active: true, currentWindow: true }, async (tabs) => {
        if (!tabs[0]) return;

        const tempRule = {
            id: 'live-preview',
            urlPatterns: ['*'],
            css,
            enabled: true
        }

        let { rules } = await chrome.storage.local.get('rules');
        if (!rules) rules = [];

        rules = rules.filter(r => r.id !== 'live-preview');
        rules.push(tempRule);
        await chrome.storage.local.set({ rules });
    });
}

function removeLivePreview() {
    chrome.storage.local.get('rules', ({ rules }) => {
        if (!rules) return;
        rules = rules.filter(r => r.id !== 'live-preview');
        chrome.storage.local.set({ rules });
    });
}

(async () => {
    const { rules } = await chrome.storage.local.get('rules');
    if (rules) renderRules(rules);
})();
