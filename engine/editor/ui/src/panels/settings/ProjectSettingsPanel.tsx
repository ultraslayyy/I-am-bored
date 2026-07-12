import { useState } from "react";
import { useEditorStore } from "../../core/store";
import RendererTab from './RendererTab';

type Tab = "project" | "graphics" | "renderer" | "window" | "build";

function SettingField({
  label,
  value,
  onChange
}: {
  label: string;
  value: any;
  onChange: (v: any) => void;
}) {
  return (
    <div style={{ marginBottom: 8 }}>
      <div style={{ fontSize: 12, opacity: 0.8 }}>{label}</div>
      <input
        value={value}
        onChange={(e) => onChange(e.target.value)}
      />
    </div>
  );
}

function ProjectTab() {
  const settings = useEditorStore(s => s.settings);
  const update = useEditorStore(s => s.updateSettings);

  return (
    <div>
      <SettingField
        label="Project Name"
        value={settings.project.name}
        onChange={(v) => update(["project", "name"], v)}
      />

      <SettingField
        label="Version"
        value={settings.project.version}
        onChange={(v) => update(["project", "version"], v)}
      />
    </div>
  );
}

function GraphicsTab() {
  const settings = useEditorStore(s => s.settings);
  const update = useEditorStore(s => s.updateSettings);

  return (
    <div>
      <SettingField
        label="Width"
        value={settings.graphics.width}
        onChange={(v) => update(["graphics", "width"], Number(v))}
      />

      <SettingField
        label="Height"
        value={settings.graphics.height}
        onChange={(v) => update(["graphics", "height"], Number(v))}
      />

      <SettingField
        label="Target FPS"
        value={settings.graphics.targetFPS}
        onChange={(v) => update(["graphics", "targetFPS"], Number(v))}
      />
    </div>
  );
}

function WindowTab() {
  const settings = useEditorStore(s => s.settings);
  const update = useEditorStore(s => s.updateSettings);

  return (
    <div>
      <label>
        <input
          type="checkbox"
          checked={settings.window.resizable}
          onChange={(e) =>
            update(["window", "resizable"], e.target.checked)
          }
        />
        Resizable
      </label>

      <label>
        <input
          type="checkbox"
          checked={settings.window.borderless}
          onChange={(e) =>
            update(["window", "borderless"], e.target.checked)
          }
        />
        Borderless
      </label>
    </div>
  );
}

function BuildTab() {
  const settings = useEditorStore(s => s.settings);
  const update = useEditorStore(s => s.updateSettings);

  return (
    <div>
      <SettingField
        label="Output Dir"
        value={settings.build.outputDir}
        onChange={(v) => update(["build", "outputDir"], v)}
      />

      <SettingField
        label="Optimization"
        value={settings.build.optimizationLevel}
        onChange={(v) => update(["build", "optimizationLevel"], v)}
      />
    </div>
  );
}

export default function ProjectSettingsPanel() {
  const [tab, setTab] = useState<Tab>("project");

  return (
    <div className="panel settings" style={{ color: 'white' }}>
      <h3>Project Settings</h3>

      <div className="settings-tabs">
        <TabButton active={tab === "project"} onClick={() => setTab("project")}>Project</TabButton>
        <TabButton active={tab === "graphics"} onClick={() => setTab("graphics")}>Graphics</TabButton>
        <TabButton active={tab === "renderer"} onClick={() => setTab("renderer")}>Renderer</TabButton>
        <TabButton active={tab === "window"} onClick={() => setTab("window")}>Window</TabButton>
        <TabButton active={tab === "build"} onClick={() => setTab("build")}>Build</TabButton>
      </div>

      <div className="settings-content">
        {tab === "project" && <ProjectTab />}
        {tab === "graphics" && <GraphicsTab />}
        {tab === "renderer" && <RendererTab />}
        {tab === "window" && <WindowTab />}
        {tab === "build" && <BuildTab />}
      </div>
    </div>
  );
}

function TabButton({ active, onClick, children }: any) {
  return (
    <button
      onClick={onClick}
      style={{
        padding: "4px 8px",
        background: active ? "#444" : "transparent",
        color: "white",
        border: "none",
        cursor: "pointer"
      }}
    >
      {children}
    </button>
  );
}