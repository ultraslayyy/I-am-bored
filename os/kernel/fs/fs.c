#include "fs.h"
#include <io/kernel_io.h>
#include <lib/string.h>
#include "cwd.h"

#include "dev.h"
#include "procfs.h"
#if __has_include("kfs.h")
    #include "kfs.h"
    #define KFS_AVAILABLE 1
#else
    #define KFS_AVAILABLE 0
#endif

fs_node_t *fs_find(const char *path);

// Nodes

static fs_node_t *root_children[] = {
#if KFS_AVAILABLE
    &dir_kernel,
#endif
    &dir_proc,
    &dir_dev
};

static fs_node_t fs_root = {
    .name = "/",
    .type = FS_DIR,
    .children = root_children,
    .child_count = ARRAY_LEN(root_children)
};

// API

void fs_init(void) {
    
}

void fs_ls(const char *path) {
    fs_node_t *node = fs_find(path);
    if (!node || node->type != FS_DIR) {
        put_string("Not a directory\n", DEFAULT_ATTR);
        return;
    }

    for (size_t i = 0; i < node->child_count;++i) {
        char buf[128];
        snprintf(buf, sizeof(buf), "%s\n", node->children[i]->name);
        put_string(buf, DEFAULT_ATTR);
    }
}

void fs_cat(const char *path) {
    fs_node_t *node = fs_find(path);
    if (!node || node->type != FS_FILE) {
        put_string("Not a file\n", DEFAULT_ATTR);
        return;
    }

    if (node->read) {
        char buffer[512];
        size_t len = node->read(buffer, sizeof(buffer) - 1);
        buffer[len] = 0;
        put_string(buffer, DEFAULT_ATTR);
        return;
    }

    if (node->content) {
        put_string(node->content, DEFAULT_ATTR);
        return;
    }

    put_string("(empty)\n", DEFAULT_ATTR);
}

static fs_node_t *fs_find_child(fs_node_t *dir, const char *name) {
    if (!dir || dir->type != FS_DIR)
        return 0;

    for (size_t i = 0; i < dir->child_count; ++i) {
        if (strcmp(dir->children[i]->name, name) == 0) {
            return dir->children[i];
        }
    }
    return 0;
}

fs_node_t *fs_find(const char *path) {
    if (!path || path[0] == 0) {
        return 0;
    }

    fs_node_t *current = &fs_root;

    if (*path == '/') {
        path++;
    }

    while (*path) {
        char *next = strchr(path, '/');
        char saved = 0;

        if (next) {
            saved = *next;
            *next = 0;
        }

        current = fs_find_child(current, path);

        if (next) {
            *next = saved;
            path = next + 1;
        } else {
            break;
        }

        if (!current) {
            return 0;
        }
    }

    return current;
}

int resolve_path(char out[MAX_PATH_LEN], const char *in) {
    if (!in || !*in) {
        return -1;
    }

    if (in[0] == '/') {
        strlcpy(out, in, MAX_PATH_LEN);
    } else if (strcmp(g_cwd, "/") == 0) {
        snprintf(out, MAX_PATH_LEN, "/%s", in);
    } else {
        snprintf(out, MAX_PATH_LEN, "%s/%s", g_cwd, in);
    }

    return 0;
}

int cmd_ls(int argc, char **argv) {
    if (argc == 1) {
        fs_ls(g_cwd);
        return 0;
    } else if (argc == 2) {
        char path[MAX_PATH_LEN];
        if (resolve_path(path, argv[1]) != 0) {
            put_string("Invalid path\n", DEFAULT_ATTR);
            return 1;
        }

        fs_ls(path);
        return 0;
    } else {
        put_string("Too many parameters\n", DEFAULT_ATTR);
        return 1;
    }
}

int cmd_cat(int argc, char **argv) {
    if (argc == 2) {
        char path[MAX_PATH_LEN];
        if (resolve_path(path, argv[1]) != 0) {
            put_string("Invalid path\n", DEFAULT_ATTR);
            return 1;
        }

        fs_cat(path);
        return 0;
    } else {
        put_string("Invalid parameters\n", DEFAULT_ATTR);
        return 1;
    }
}