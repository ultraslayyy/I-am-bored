// Learn more about Tauri commands at https://tauri.app/develop/calling-rust/
use std::fs;
use std::path::Path;
use std::process::Command;

use tauri::command;

#[command]
fn open_in_default_editor(path: String) -> Result<(), String> {
    /*
    Command::new("cmd")
        .args(["/C", "start", "", &path])
        .spawn()
        .map_err(|e| e.to_string())?;
    */
    Command::new("explorer")
        .arg(&path)
        .spawn()
        .map_err(|e| e.to_string())?;

    Ok(())
}

#[command]
fn delete_path(path: String) -> Result<(), String> {
    let p = Path::new(&path);

    if p.is_dir() {
        fs::remove_dir_all(p).map_err(|e| e.to_string())?;
    } else {
        fs::remove_file(p).map_err(|e| e.to_string())?;
    }

    Ok(())
}

#[command]
fn rename_path(path: String, new_name: String) -> Result<String, String> {
    let p = Path::new(&path);
    let parent = p.parent().ok_or("No parent")?;

    let new_path = parent.join(new_name);

    fs::rename(p, &new_path).map_err(|e| e.to_string())?;

    Ok(new_path.to_string_lossy().to_string())
}

#[command]
fn read_dir(path: String) -> Result<Vec<(String, String, bool)>, String> {
    let mut result = Vec::new();

    for entry in fs::read_dir(path).map_err(|e| e.to_string())? {
        let entry = entry.map_err(|e| e.to_string())?;
        let path = entry.path();

        let name = path.file_name().unwrap_or_default().to_string_lossy().to_string();

        let is_dir = path.is_dir();

        result.push((name, path.to_string_lossy().to_string(), is_dir));
    }

    Ok(result)
}

#[command]
fn create_project_structure(root: String) -> Result<(), String> {
    fs::create_dir_all(format!("{}/assets", root)).map_err(|e| e.to_string())?;
    fs::create_dir_all(format!("{}/scenes", root)).map_err(|e| e.to_string())?;
    fs::create_dir_all(format!("{}/scripts", root)).map_err(|e| e.to_string())?;
    Ok(())
}

#[command]
fn ensure_project_structure(root: String) -> Result<(), String> {
    fs::create_dir_all(format!("{}/assets", root)).map_err(|e| e.to_string())?;
    fs::create_dir_all(format!("{}/scenes", root)).map_err(|e| e.to_string())?;
    fs::create_dir_all(format!("{}/scripts", root)).map_err(|e| e.to_string())?;
    Ok(())
}

#[command]
fn save_project(path: String, data: String) -> Result<(), String> {
    fs::write(path, data).map_err(|e| e.to_string())?;
    Ok(())
}

#[command]
fn load_project(path: String) -> Result<String, String> {
    let content = fs::read_to_string(path).map_err(|e| e.to_string())?;
    Ok(content)
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
        .plugin(tauri_plugin_opener::init())
        .plugin(tauri_plugin_dialog::init())
        .invoke_handler(tauri::generate_handler![
            save_project,
            load_project,
            ensure_project_structure,
            create_project_structure,
            read_dir,
            open_in_default_editor,
            rename_path,
            delete_path
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
