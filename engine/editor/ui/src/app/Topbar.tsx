import { useState, useEffect } from 'react';
import { useEditorStore } from '../core/store';
import { saveCurrentProject, openProject, saveProjectAs } from '../core/ProjectManager';

export default function Topbar() {
    const [openMenu, setOpenMenu] = useState<string | null>(null);
    const setActivePanel = useEditorStore(s => s.setActivePanel);

    const toggle = (menu: string) => {
        setOpenMenu(openMenu === menu ? null : menu);
    }

    const openProjectSetings = () => {
        setActivePanel('projectSettings');
        setOpenMenu(null);
    }

    const openBuildSettings = () => {
        setActivePanel('buildSettings');
        setOpenMenu(null);
    }

    const openScene = () => {
        setActivePanel('scene');
        setOpenMenu(null);
    }

    useEffect(() => {
        const handler = (e: KeyboardEvent) => {
            if (e.ctrlKey && e.key.toLowerCase() === 's') {
                e.preventDefault();
                if (e.shiftKey) {
                    saveProjectAs();
                } else {
                    saveCurrentProject();
                }
            }
        }

        window.addEventListener('keydown', handler);
        return () => window.removeEventListener('keydown', handler);
    });

    return (
        <div className="topbar">
            <div className="menu">
                <button onClick={() => toggle("file")}>File</button>
                <button onClick={() => toggle("edit")}>Edit</button>
                <button onClick={() => toggle("project")}>Project</button>
                <button onClick={() => toggle("view")}>View</button>
                <button onClick={() => toggle("settings")}>Settings</button>
                <button onClick={() => toggle("help")}>Help</button>
            </div>

            {openMenu && (
                <div className="dropdown">
                    {openMenu === "file" && (
                        <>
                            <div onClick={() => {
                                setActivePanel('projectWizard');
                                setOpenMenu(null);
                            }}>New Project</div>
                            <div onClick={() => {
                                openProject();
                                setOpenMenu(null);
                            }}>Open Project</div>
                            <div onClick={() => {
                                saveCurrentProject();
                                setOpenMenu(null);
                            }}>Save</div>
                            <div onClick={() => {
                                saveProjectAs();
                                setOpenMenu(null);
                            }}>Save As</div>
                        </>
                    )}

                    {openMenu === "project" && (
                        <>
                            <div onClick={openProjectSetings}>Project Settings</div>
                            <div onClick={openBuildSettings}>Build Settings</div>
                        </>
                    )}

                    {openMenu === "settings" && (
                        <>
                            <div>Graphics</div>
                            <div>Editor</div>
                        </>
                    )}

                    {openMenu === 'view' && (
                        <>
                            <div onClick={openScene}>Scene View</div>
                        </>
                    )}
                </div>
            )}
        </div>
    );
}