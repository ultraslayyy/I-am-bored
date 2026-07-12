import HierarchyPanel from '../panels/HierarchyPanel';
import InspectorPanel from '../panels/InspectorPanel';
import ScenePanel from '../panels/ScenePanel';
import ConsolePanel from '../panels/ConsolePanel';
import AssetsPanel from '../panels/AssetsPanel';
import Topbar from './Topbar';
import { useEditorStore } from '../core/store';
import ProjectSettingsPanel from '../panels/settings/ProjectSettingsPanel';
import ProjectWizard from '../panels/wizard/ProjectWizard';

export default function EditorLayout() {
    const activePanel = useEditorStore(s => s.activePanel);
    const bottomTab = useEditorStore(s => s.bottomTab);
    const setBottomTab = useEditorStore(s => s.setBottomTab);

    return (
        <div className='editor-root'>
            <Topbar />

            <div className='main'>
                <div className='left'>
                    <HierarchyPanel />
                </div>

                <div className='center'>
                    {activePanel === 'scene' && <ScenePanel />}
                    {activePanel === 'projectSettings' && <ProjectSettingsPanel />}
                    {activePanel === 'projectWizard' && <ProjectWizard />}
                </div>

                <div className='right'>
                    <InspectorPanel />
                </div>
            </div>

            <div className='bottom'>
                <div style={{ display: 'flex', gap: 4, padding: 4, borderBottom: '1px solid #333' }}>
                    {['console', 'assets'].map(tab => (
                        <button
                            key={tab}
                            onClick={() => setBottomTab(tab as any)}
                            style={{
                                padding: '3px 8px',
                                background: bottomTab === tab ? '#555' : '#333',
                                color: 'white',
                                border: 'none',
                                fontSize: 12
                            }}
                        >
                            {tab}
                        </button>
                    ))}
                </div>

                <div style={{ height: 'calc(100% - 28px)' }}>
                    {bottomTab === 'console' && <ConsolePanel />}
                    {bottomTab === 'assets' && <AssetsPanel />}
                </div>
            </div>
        </div>
    );
}