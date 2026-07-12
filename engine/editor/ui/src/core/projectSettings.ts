export type RendererBackend =
    | 'legacy_gdi'
    | 'opengl_1_1'
    | 'opengl_3_3'
    | 'directx11'
    | 'directx12'
    | 'direct2d';

export interface OpenGLSettings {
    version: '1.1' | '3.3';
}

export interface DirectXSettings {
    featureLevel: '11_0' | '12_0';
}

export interface RendererSettings {
    opengl: OpenGLSettings;
    directx: DirectXSettings;

    vsync: boolean;
    antialiasing: boolean;
}

export interface ProjectSettings {
    project: {
        name: string;
        version: string;
    };

    graphics: {
        width: number;
        height: number;
        fullscreen: boolean;
        pixelRatio: number;
        targetFPS: number;
    };

    renderer: {
        enabledBackends: RendererBackend[];
        defaultBackend: RendererBackend;
        fallbackOrder: RendererBackend[];

        settings: RendererSettings;
    };

    window: {
        resizable: boolean;
        borderless: boolean;
    };

    build: {
        outputDir: string;
        optimizationLevel: 'debug' | 'release';
    };
}

export const defaultProjectSettings: ProjectSettings = {
    project: { name: 'New Project', version: '0.1.0' },

    graphics: {
        width: 1280,
        height: 720,
        fullscreen: false,
        pixelRatio: 1,
        targetFPS: 60
    },

    renderer: {
        enabledBackends: ['directx11', 'opengl_3_3'],
        defaultBackend: 'directx11',
        fallbackOrder: ['directx11', 'opengl_3_3', 'legacy_gdi'],

        settings: {
            opengl: {
                version: '1.1'
            },
            directx: {
                featureLevel: '11_0'
            },
            vsync: false,
            antialiasing: true
        }
    },

    window: {
        resizable: true,
        borderless: false
    },

    build: {
        outputDir: 'build/',
        optimizationLevel: 'debug'
    }
};