import os

ROOT_DIR = "kernel"
OUT_FILE = "kernel/fs/vfs.c"

def sanitize_name(name):
    return name.replace('.', '_').replace('-', '_').replace('/', '_').replace('\\', '_')

def escape_c_string(s: str) -> str:
    s = s.replace('\\', '\\\\')
    s = s.replace('"', '\\"')
    s = s.replace('\n', '\\n')
    return s

def generate_file_node(path, name):
    with open(path, "r") as f:
        content = f.read()
    size = len(content)
    cname = sanitize_name(path)
    escaped_content = escape_c_string(content)
    c = f'static fs_node_t file_{cname} = {{\n'
    c += f'    .name = "{name}",\n'
    c += f'    .type = FS_FILE,\n'
    c += f'    .content = "{escaped_content}",\n'
    c += f'    .size = {size}\n'
    c += '};\n\n'
    return c, f'&file_{cname}'

def generate_dir_node(name, children_ptrs):
    c_name = sanitize_name(name)
    c = f'static fs_node_t *dir_{c_name}_children[] = {{ {", ".join(children_ptrs)} }};\n'
    c += f'static fs_node_t dir_{c_name} = {{\n'
    c += f'    .name = "{name}",\n'
    c += f'    .type = FS_DIR,\n'
    c += f'    .children = dir_{c_name}_children,\n'
    c += f'    .child_count = {len(children_ptrs)}\n'
    c += '};\n\n'
    return c, f'&dir_{c_name}'

def walk_dir(path):
    file_nodes = []
    dir_nodes = []

    children_ptrs = []

    for entry in sorted(os.listdir(path)):
        full_path = os.path.join(path, entry)
        if os.path.isdir(full_path):
            c, ptr = walk_dir(full_path)
            dir_nodes.append(c)
            children_ptrs.append(ptr)
        else:
            c, ptr = generate_file_node(full_path, entry)
            file_nodes.append(c)
            children_ptrs.append(ptr)

    c_dir, ptr_dir = generate_dir_node(os.path.basename(path), children_ptrs)
    return "\n".join(dir_nodes + file_nodes + [c_dir]), ptr_dir

all_nodes, root_ptr = walk_dir(ROOT_DIR)

kernel_c = f'static fs_node_t *kernel_children[] = {{ {root_ptr} }};\n'
kernel_c += 'static fs_node_t dir_kernel = {\n'
kernel_c += '    .name = "kernel",\n'
kernel_c += '    .type = FS_DIR,\n'
kernel_c += '    .children = kernel_children,\n'
kernel_c += f'    .child_count = {1}\n'
kernel_c += '};'

with open(OUT_FILE, "w") as f:
    f.write('#include "fs.h"\n')
    f.write(all_nodes)
    # f.write(kernel_c)

print(f"FS C file written to {OUT_FILE}")
