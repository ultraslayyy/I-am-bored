import { useState } from 'react';
import { useEditorStore } from '../../core/store';
import { RendererBackend } from '../../core/projectSettings';

const BACKENDS: { id: RendererBackend; label: string }[] = [
    { id: 'directx11', label: 'DirectX 11' },
    { id: 'directx12', label: 'DirectX 12' },
    { id: 'opengl_3_3', label: 'OpenGL 3.3' },
    { id: 'opengl_1_1', label: 'OpenGL 1.1' },
    { id: 'direct2d', label: 'Direct2D' },
    { id: 'legacy_gdi', label: 'Legacy GDI' }
];

export default function RendererTab() {
    const settings = useEditorStore(s => s.settings);
    const update = useEditorStore(s => s.updateSettings);

    const [activeTab, setActiveTab] = useState<RendererBackend | null>(null);

    const enabled = settings.renderer.enabledBackends;
    const defaultBackend = settings.renderer.defaultBackend;
    const fallbackOrder = settings.renderer.fallbackOrder;

    const toggleBackend = (id: RendererBackend) => {
        let newList: RendererBackend[];

        if (enabled.includes(id)) {
            newList = enabled.filter(b => b !== id);
        } else {
            newList = [...enabled, id];
        }

        if (newList.length === 0) return;

        update(['renderer', 'enabledBackends'], newList);

        // fix default if removed
        if (!newList.includes(defaultBackend)) {
            update(['renderer', 'defaultBackend'], newList[0]);
        }

        // auto-add/remove from fallback order
        update(['renderer', 'fallbackOrder'], [
            ...fallbackOrder.filter(b => newList.includes(b)),
            ...newList.filter(b => !fallbackOrder.includes(b))
        ]);

        if (!activeTab && newList.length > 0) {
            setActiveTab(newList[0]);
        }
    };

    const setDefault = (id: RendererBackend) => {
        if (!enabled.includes(id)) return;
        update(['renderer', 'defaultBackend'], id);
    };

    // drag reorder fallback
    const moveFallback = (from: number, to: number) => {
        const arr = [...fallbackOrder];
        const [item] = arr.splice(from, 1);
        arr.splice(to, 0, item);

        update(['renderer', 'fallbackOrder'], arr);
    };

    return (
        <div>
            <h4>Renderer Backends</h4>

            {/* CHECKLIST */}
            <div style={{ marginBottom: 10 }}>
                {BACKENDS.map(b => {
                    const isEnabled = enabled.includes(b.id);
                    const isDefault = defaultBackend === b.id;

                    return (
                        <div
                            key={b.id}
                            style={{
                                display: 'flex',
                                alignItems: 'center',
                                gap: 8,
                                padding: '3px 0'
                            }}
                        >
                            <input
                                type='checkbox'
                                checked={isEnabled}
                                onChange={() => toggleBackend(b.id)}
                            />

                            <span style={{ flex: 1 }}>{b.label}</span>

                            <button
                                onClick={() => setDefault(b.id)}
                                disabled={!isEnabled}
                                style={{
                                    fontSize: 11,
                                    padding: '2px 6px',
                                    background: isDefault ? '#4caf50' : '#333',
                                    color: 'white',
                                    border: 'none',
                                    opacity: isEnabled ? 1 : 0.4,
                                    cursor: isEnabled ? 'pointer' : 'not-allowed'
                                }}
                            >
                                {isDefault ? 'Default' : 'Set'}
                            </button>
                        </div>
                    );
                })}
            </div>

            {/* FALLBACK ORDER (DRAG STYLE UI) */}
            <h4 style={{ marginTop: 12 }}>Fallback Order</h4>

            <div style={{ fontSize: 12, opacity: 0.7, marginBottom: 6 }}>
                Drag order (top = highest priority)
            </div>

            {fallbackOrder.map((b, i) => {
                const label = BACKENDS.find(x => x.id === b)?.label ?? b;

                return (
                    <div
                        key={b}
                        draggable
                        onDragStart={(e) => {
                            e.dataTransfer.setData('from', i.toString());
                        }}
                        onDragOver={(e) => e.preventDefault()}
                        onDrop={(e) => {
                            const from = Number(e.dataTransfer.getData('from'));
                            moveFallback(from, i);
                        }}
                        style={{
                            padding: '4px',
                            marginBottom: 4,
                            background: '#2a2a2a',
                            cursor: 'grab'
                        }}
                    >
                        {label}
                    </div>
                );
            })}

            {/* TABS FOR ENABLED BACKENDS */}
            <h4 style={{ marginTop: 12 }}>Active Renderer Settings</h4>

            <div style={{ display: 'flex', gap: 4, marginBottom: 8 }}>
                {enabled.map(b => (
                    <button
                        key={b}
                        onClick={() => setActiveTab(b)}
                        style={{
                            padding: '3px 6px',
                            background: activeTab === b ? '#555' : '#333',
                            color: 'white',
                            border: 'none',
                            fontSize: 11
                        }}
                    >
                        {BACKENDS.find(x => x.id === b)?.label}
                    </button>
                ))}
            </div>

            {/* SETTINGS PANEL */}
            <div style={{ padding: 8, background: '#1f1f1f' }}>
                {!activeTab && <div>Select a renderer</div>}

                {activeTab && (
                    <>
                        <h5 style={{ marginTop: 0 }}>
                            {BACKENDS.find(x => x.id === activeTab)?.label}
                        </h5>

                        {/* OPENGL SETTINGS EXAMPLE */}
                        {(activeTab === 'opengl_1_1' || activeTab === 'opengl_3_3') && (
                            <div>
                                <label>
                                    OpenGL Version:
                                    <select
                                        value={activeTab === 'opengl_3_3' ? '3.3' : '1.1'}
                                        onChange={(e) => {
                                            const val = e.target.value;

                                            const newBackend =
                                                val === '3.3' ? 'opengl_3_3' : 'opengl_1_1';

                                            if (enabled.includes(newBackend)) {
                                                setActiveTab(newBackend);
                                                update(['renderer', 'defaultBackend'], newBackend);
                                            }
                                        }}
                                    >
                                        <option value='1.1'>1.1</option>
                                        <option value='3.3'>3.3</option>
                                    </select>
                                </label>
                            </div>
                        )}

                        {/* PLACEHOLDER FOR FUTURE BACKENDS */}
                        {activeTab.startsWith('directx') && (
                            <div style={{ fontSize: 12, opacity: 0.7 }}>
                                DirectX settings coming soon...
                            </div>
                        )}

                        {activeTab === 'legacy_gdi' && (
                            <div style={{ fontSize: 12, opacity: 0.7 }}>
                                Legacy renderer (no config)
                            </div>
                        )}
                    </>
                )}
            </div>
        </div>
    );
}