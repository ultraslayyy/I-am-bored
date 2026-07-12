import { ProjectSettings } from './projectSettings';
import { Scene } from './scene';

export interface ProjectFile {
    settings: ProjectSettings;
    scene: Scene;

    metadata: {
        name: string;
        version: string;
        lastSaved: number;
    }
}