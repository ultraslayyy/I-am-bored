import { Entity, Scene, createTestScene } from './scene';
import { ProjectSettings, defaultProjectSettings } from './projectSettings';
import { create } from 'zustand';
import { ProjectFile } from './projectFile';

const generateId = () => crypto.randomUUID();

interface EditorState {
    scene: Scene;
    selectedEntityId: string | null;

    selectEntity: (id: string) => void;
    updateComponent: (entityId: string, compIndex: number, key: string, value: any) => void;

    renameEntity: (id: string, name: string) => void;
    addEntity: () => void;
    deleteEntity: (id: string) => void;

    deselectEntity: () => void;

    settings: ProjectSettings;

    setSettings: (settings: ProjectSettings) => void;
    updateSettings: (path: string[], value: any) => void;

    activePanel: string;
    setActivePanel: (panel: string) => void;

    currentProjectPath: string | null;
    setProjectPath: (path: string | null) => void;

    projectRootPath: string | null;
    paths: {
        assets: string | null;
        scenes: string | null;
        scripts: string | null;
    }

    setProjectRoot: (root: string) => void;

    loadProject: (project: ProjectFile, path: string) => void;

    bottomTab: 'console' | 'assets';
    setBottomTab: (tab: 'console' | 'assets') => void;

    selectedAssetPath: string | null;
    selectAsset: (path: string | null) => void;
}

export const useEditorStore = create<EditorState>((set) => ({
    scene: createTestScene(),
    selectedEntityId: null,
    selectedAssetPath: null,
    settings: defaultProjectSettings,
    activePanel: 'scene',
    currentProjectPath: null,
    projectRootPath: null,
    paths: {
        assets: null,
        scenes: null,
        scripts: null
    },
    bottomTab: 'console' as 'console' | 'assets',

    selectEntity: (id) => set({ selectedEntityId: id }),

    updateComponent: (entityId, compIndex, key, value) => set((state) => {
        const scene = { ...state.scene }
        const entity = scene.entities.find(e => e.id === entityId);
        if (!entity) return state;

        entity.components[compIndex].data[key] = value;
        return { scene }
    }),

    renameEntity: (id, name) => set((state) => {
        const scene = { ...state.scene }
        const entity = scene.entities.find(e => e.id === id);
        if (!entity) return state;

        entity.name = name;
        return { scene }
    }),
    
    addEntity: () => set((state) => {
        const scene = { ...state.scene }

        const newEntity: Entity = {
            id: generateId(),
            name: 'New Entity',
            components: [
                {
                    type: 'Transform',
                    data: {
                        x: 0,
                        y: 0,
                        rotation: 0
                    }
                }
            ]
        }

        scene.entities.push(newEntity);

        return { scene, selectedEntityId: newEntity.id }
    }),

    deleteEntity: (id) => set((state) => {
        const scene = { ...state.scene }

        scene.entities = scene.entities.filter(e => e.id !== id);

        return { scene, selectedEntityId: null }
    }),

    deselectEntity: () => set((state) => {
        return { scene: state.scene, selectedEntityId: null }
    }),

    setSettings: (settings) => set({ settings }),

    updateSettings: (path, value) => set((state) => {
        const settings = structuredClone(state.settings);

        let obj: any = settings;
        for (let i = 0; i < path.length - 1; i++) {
            obj = obj[path[i]];
        }

        obj[path[path.length - 1]] = value;

        return { settings }
    }),

    setActivePanel: (panel) => set({ activePanel: panel }),

    setProjectPath: (path) => set({ currentProjectPath: path }),

    loadProject: (project, path) => set({
        settings: project.settings,
        scene: project.scene,
        selectedEntityId: null,
        currentProjectPath: path
    }),

    setProjectRoot: (root) => set({
        projectRootPath: root,
        paths: {
            assets: `${root}/assets`,
            scenes: `${root}/scenes`,
            scripts: `${root}/scripts`            
        }
    }),

    setBottomTab: (tab) => set({ bottomTab: tab }),

    selectAsset: (path) => set({
        selectedAssetPath: path,
        selectedEntityId: null
    })
}));