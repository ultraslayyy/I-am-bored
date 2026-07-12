export interface Component {
    type: string;
    data: Record<string, any>;
}

export interface Entity {
    id: string;
    name: string;
    components: Component[];
}

export interface Scene {
    version: string;
    name: string;
    entities: Entity[];
}

export const createTestScene = (): Scene => ({
    version: '0.1',
    name: 'Test Scene',
    entities: [
        {
            id: '1',
            name: 'Player',
            components: [
                {
                    type: 'Transform',
                    data: {
                        x: 0,
                        y: 0,
                        rotation: 0
                    }
                },
                {
                    type: 'Rect',
                    data: {
                        width: 100,
                        height: 60,
                        color: '#ff5555'
                    }
                }
            ]
        },
        {
            id: '2',
            name: 'Camera',
            components: [
                {
                    type: 'Transform',
                    data: {
                        x: 100,
                        y: 100,
                        rotation: 0
                    }
                }
            ]
        }
    ]
});