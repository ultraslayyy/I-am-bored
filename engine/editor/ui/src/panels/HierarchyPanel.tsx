import { useEffect, useState } from 'react';
import { useEditorStore } from '../core/store';

export default function HierarchyPanel() {
    const scene = useEditorStore(s => s.scene);
    const select = useEditorStore(s => s.selectEntity);
    const selectedId = useEditorStore(s => s.selectedEntityId);
    const rename = useEditorStore(s => s.renameEntity);
    const addEntity = useEditorStore(s => s.addEntity);
    const deleteEntity = useEditorStore(s => s.deleteEntity);
    const deselect = useEditorStore(s => s.deselectEntity);

    const [editingId, setEditingId] = useState<string | null>(null);
    const [tempName, setTempName] = useState('');

    useEffect(() => {
        const handler = (e: KeyboardEvent) => {
            if (e.key === 'Delete' && selectedId) {
                deleteEntity(selectedId);
            }

            if (e.key === 'F2' && selectedId) {
                const entity = scene.entities.find(e => e.id === selectedId);
                if (entity) {
                    setEditingId(entity.id);
                    setTempName(entity.name);
                }
            }

            if (e.key === 'Escape' && selectedId) {
                deselect();
                if (editingId) setEditingId(null);
            }
        }

        window.addEventListener('keydown', handler);
        return () => window.removeEventListener('keydown', handler);
    }), [selectedId, scene];

    return (
        <div className='panel'>
            <h3>Hierarchy</h3>

            <div style={{ marginBottom: 8 }}>
                <button onClick={addEntity}>+</button>
                <button onClick={() => selectedId && deleteEntity(selectedId)}>-</button>
            </div>

            {scene.entities.map(e => {
                const isEditing = editingId === e.id;

                return (
                    <div
                        key={e.id}
                        className={`entity ${selectedId === e.id ? 'selected' : ''}`}
                        onClick={() => select(e.id)}
                        onDoubleClick={() => {
                            setEditingId(e.id);
                            setTempName(e.name)
                        }}
                    >
                        {isEditing ? (
                            <input
                                autoFocus
                                value={tempName}
                                onChange={(ev) => setTempName(ev.target.value)}
                                onBlur={() => {
                                    rename(e.id, tempName);
                                    setEditingId(null);
                                }}
                                onKeyDown={(ev) => {
                                    if (ev.key === 'Enter') {
                                        rename(e.id, tempName);
                                        setEditingId(null);
                                    }
                                }}
                            />
                        ) : (
                            e.name
                        )}
                    </div>
                );
            })}
        </div>
    );
}