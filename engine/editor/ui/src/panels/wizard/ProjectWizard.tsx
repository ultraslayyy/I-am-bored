import { useState } from 'react';
import { open } from '@tauri-apps/plugin-dialog';
import { useEditorStore } from '../../core/store';
import { createNewProject } from '../../core/ProjectManager';

export default function ProjectWizard() {
    const setActivePanel = useEditorStore(s => s.setActivePanel);

    const [projectName, setProjectName] = useState('New Project');
    const [folder, setFolder] = useState<string | null>(null);

    const pickFolder = async () => {
        const selected = await open({
            directory: true,
            multiple: false
        });

        if (!selected) return;

        setFolder(selected);
    }

    const create = async () => {
        if (!folder) return;

        await createNewProject(folder, projectName);

        setActivePanel('scene');
    }

    return (
        <div style={{ padding: 20, display: 'flex', flexDirection: 'column', gap: 12 }}>
            <h2 style={{ color: 'white' }}>New Project</h2>

            <input value={projectName} onChange={(e) => setProjectName(e.target.value)} placeholder='Project Name' />
            <button onClick={pickFolder}>
                {folder ? `Folder: ${folder}` : 'Select Project Folder'}
            </button>
            <button onClick={create} disabled={!folder || !projectName}>Create Project</button>
        </div>
    );
}