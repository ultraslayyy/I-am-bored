import { useEditorStore } from './store';
import { saveProjectDialog, saveProject, loadProjectDialog, loadProject, ensureProjectStructure, createProjectStructure } from './tauri';
import { ProjectFile } from './projectFile';
import { getProjectRoot } from './path';
import { defaultProjectSettings } from './projectSettings';

export async function saveCurrentProject() {
    const state = useEditorStore.getState();
    let path = state.currentProjectPath;

    if (!path) {
        path = await saveProjectDialog();
        if (!path) return;

        state.setProjectPath(path);
    }

    const project: ProjectFile = {
        settings: state.settings,
        scene: state.scene,
        metadata: {
            name: state.settings.project.name,
            version: state.settings.project.version,
            lastSaved: Date.now()
        }
    };

    await saveProject(path, project);
}

export async function openProject() {
    const path = await loadProjectDialog();
    if (!path) return;

    const project: ProjectFile = await loadProject(path);
    const root = getProjectRoot(path);
    const store = useEditorStore.getState();

    store.loadProject(project, path);
    store.setProjectRoot(root);

    ensureProjectStructure(root);
}

export async function saveProjectAs() {
    const path = await saveProjectDialog();
    if (!path) return;

    const root = getProjectRoot(path);

    const state = useEditorStore.getState();
    state.setProjectPath(path);
    state.setProjectRoot(root);

    ensureProjectStructure(root);

    await saveCurrentProject();
}

export async function createNewProject(folder: string, name: string) {
    const root = `${folder}/${name}`;

    await createProjectStructure(root);

    const project: ProjectFile = {
        settings: {
            ...defaultProjectSettings,
            project: {
                name,
                version: '0.1.0'
            }
        },
        scene: {
            name: 'Main scene',
            version: '0.1',
            entities: []
        },
        metadata: {
            name,
            version: '0.1.0',
            lastSaved: Date.now()
        }
    }

    const path = `${root}/project.uprj`;

    await saveProject(path, JSON.stringify(project, null, 2));

    const store = useEditorStore.getState();

    store.setProjectRoot(root);
    store.setProjectPath(path);
    store.loadProject(project, path);

    store.setActivePanel('scene');
}