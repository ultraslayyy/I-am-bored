import { invoke } from '@tauri-apps/api/core';
import { open, save } from '@tauri-apps/plugin-dialog';

export async function saveProjectDialog(): Promise<string | null> {
    return await save({
        filters: [
            {
                name: 'Project JSON',
                extensions: ['uprj']
            }
        ]
    });
}

export async function saveProject(path: string, data: any) {
    return await invoke('save_project', { path, data: JSON.stringify(data, null, 2) });
}

export async function loadProjectDialog(): Promise<string | null> {
    const filePath = await open({
        multiple: false,
        directory: false,
        filters: [
            {
                name: 'Project file',
                extensions: ['uprj']
            }
        ]
    });

    return filePath;
}

export async function loadProject(path: string): Promise<any> {
    const result = await invoke<string>('load_project', { path });
    return JSON.parse(result);
}

export async function ensureProjectStructure(root: string) {
    return invoke('ensure_project_structure', { root });
}

export async function createProjectStructure(root: string) {
    return invoke('create_project_structure', { root });
}

export async function readDir(path: string) {
    const result = await invoke<[string, string, boolean][]>('read_dir', { path });

    return result.map(([name, path, isDir]) => ({
        name,
        path,
        isDir
    }));
}

export async function openInDefaultEditor(path: string) {
    return invoke('open_in_default_editor', { path });
}

export async function deletePath(path: string) {
    await invoke('delete_path', { path });
}

export async function renamePath(path: string, newName: string): Promise<string> {
    return await invoke('rename_path', { path, new_name: newName });
}