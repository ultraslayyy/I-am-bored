import { useEditorStore } from '../core/store';

export default function InspectorPanel() {
    const scene = useEditorStore(s => s.scene);
    const selectedId = useEditorStore(s => s.selectedEntityId);
    const update = useEditorStore(s => s.updateComponent);

    const entity = scene.entities.find(e => e.id === selectedId);
    if (!entity) return <div className='panel'>No selection</div>;

    return (
        <div className='panel'>
            <h3>{entity.name}</h3>

            {entity.components.map((c, i) => (
                <div key={i}>
                    <h4>{c.type}</h4>

                    {Object.entries(c.data).map(([key, value]) => (
                        <div key={key}>
                            {key}:{" "}
                            <input value={value} onChange={(e) => update(entity.id, i, key, Number(e.target.value))}/>
                        </div>
                    ))}
                </div>
            ))}
        </div>
    );
}