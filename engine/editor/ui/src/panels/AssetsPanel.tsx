import { useEffect, useState } from 'react';
import { useEditorStore } from '../core/store';
import { openInDefaultEditor, readDir, deletePath, renamePath } from '../core/tauri';
import { dirname } from '../core/path';

interface FileEntry {
    name: string;
    path: string;
    isDir: boolean;
}

export default function AssetsPanel() {
    const assetsPath = useEditorStore(s => s.paths.assets);
    const selectedAsset = useEditorStore(s => s.selectedAssetPath);
    const selectAsset = useEditorStore(s => s.selectAsset);

    const [entries, setEntries] = useState<FileEntry[]>([]);
    const [currentPath, setCurrentPath] = useState<string | null>(null);
    const [contextMenu, setContextMenu] = useState<{
        x: number;
        y: number;
        path: string;
    } | null>(null);
    const [editingPath, setEditingPath] = useState<string | null>(null);
    const [tempName, setTempName] = useState('');

    useEffect(() => {
        const handler = (e: KeyboardEvent) => {
            if ((e.target as HTMLElement).tagName === 'INPUT') return;
            if (e.key !== 'F2') return;

            const selected = selectedAsset;
            if (!selected) return;

            const name = selected.split('/').pop()!;
            const { base } = splitName(name);

            setEditingPath(selected);
            setTempName(base);
        }

        window.addEventListener('keydown', handler);
        return () => window.removeEventListener('keydown', handler);
    }, [selectedAsset]);

    useEffect(() => {
        if (!assetsPath) return;
        setCurrentPath(assetsPath);
    }, [assetsPath]);

    useEffect(() => {
        if (!currentPath) return;
        load(currentPath);
    }, [currentPath]);

    const load = async (path: string) => {
        const files = await readDir(path);
        setEntries(files);
    }

    const goUp = () => {
        if (!currentPath || !assetsPath) return;
        if (currentPath === assetsPath) return;

        setCurrentPath(dirname(currentPath));
    }

    const remove = async (path: string) => {
        await deletePath(path);
        load(currentPath!);
    }
    
    function splitName(full: string) {
        const lastDot = full.lastIndexOf('.');
        if (lastDot <= 0) return { base: full, ext: '' }

        return {
            base: full.slice(0, lastDot),
            ext: full.slice(lastDot)
        }
    }

    const handleRename = async (path: string, newFullName: string) => {
        if (!newFullName.trim()) return;

        try {
            const newPath = await renamePath(path, newFullName);
            await load(currentPath!);

            if (selectedAsset === path) {
                selectAsset(newPath);
            }
        } catch (e) {
            console.error('Rename failed', e);
        }
    }

    return (
        <div className='panel'>
            <h3>Assets</h3>

            <div style={{ marginBottom: 6 }}>
                <button onClick={goUp}>Up</button>
            </div>

            {entries.map(e => {
                const isEditing = editingPath === e.path;
                const isSelected = selectedAsset === e.path;

                return (
                    <div
                        key={e.path}
                        onClick={() => selectAsset(e.path)}
                        onDoubleClick={() => {
                            if (isEditing) return;
                            if (e.isDir) {
                                setCurrentPath(e.path);
                            } else {
                                openInDefaultEditor(e.path);
                            }
                        }}
                        onContextMenu={(ev) => {
                            ev.preventDefault();
                            setContextMenu({
                                x: ev.clientX,
                                y: ev.clientY,
                                path: e.path
                            })
                        }}
                        style={{
                            padding: 4,
                            cursor: 'pointer',
                            display: 'flex',
                            gap: 6,
                            userSelect: 'none',
                            background: isSelected ? '#444' : 'transperent'
                        }}
                    >
                        <span>{e.isDir ? '📁' : '📄'}</span>
                        
                        {isEditing ? (() => {
                            const fullName = e.name;
                            const { ext } = splitName(fullName);

                            return (
                                <input
                                    autoFocus
                                    value={tempName}
                                    onChange={(ev) => setTempName(ev.target.value)}
                                    onFocus={(ev) => {
                                        ev.target.setSelectionRange(0, tempName.length);
                                    }}
                                    onBlur={async () => {
                                        await handleRename(e.path, tempName + ext);
                                        setEditingPath(null);
                                    }}
                                    onKeyDown={async (ev) => {
                                        if (ev.key === 'Enter') {
                                            await handleRename(e.path, tempName + ext);
                                            setEditingPath(null);
                                        }
                                        if (ev.key === 'Escape') {
                                            setEditingPath(null);
                                        }
                                    }}
                                    style={{
                                        flex: 1,
                                        background: '#222',
                                        border: '1px solid #555',
                                        color: 'white'
                                    }}
                                />
                            );
                        })() : (
                            <span>{e.name}</span>
                        )}
                    </div>
                );
            })}

            {contextMenu && (
                <div
                    style={{
                        position: 'fixed',
                        top: contextMenu.y,
                        left: contextMenu.x,
                        background: '#2a2a2a',
                        border: '1px solid #444',
                        zIndex: 999
                    }}
                    onMouseLeave={() => setContextMenu(null)}
                >
                    <div className='context-item' onClick={() => {
                        const name = contextMenu.path.split('/').pop()!;
                        setEditingPath(contextMenu.path);
                        setTempName(name);
                        setContextMenu(null);
                    }}>Rename</div>

                    <div className='context-item' onClick={() => {
                        remove(contextMenu.path);
                        setContextMenu(null);
                    }}>Delete</div>
                </div>
            )}
        </div>
    );
}