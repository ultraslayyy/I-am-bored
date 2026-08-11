#include <lib/stddef.h>
#include <lib/stdint.h>
#include <lib/stdlib.h>
#include <lib/string.h>
#include <fs/vfs.h>
#include "fat16.h"

typedef struct {
    block_device_t *dev;

    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t  num_fats;
    uint16_t root_entries;
    uint16_t sectors_per_fat;

    uint32_t fat_start;
    uint32_t root_dir_start;
    uint32_t data_start;

    uint32_t root_dir_sectors;
} fat16_fs_t;

typedef struct {
    fat16_fs_t *fs;
    uint16_t first_cluster;
    uint32_t size;
} fat16_file_t;

typedef struct {
    char name[12];
    uint8_t  attr;
    uint16_t first_cluster;
    uint32_t size;
} fat16_dirent_t;

static filesystem_t fat16_fs;
static fat16_fs_t *fat16_current_fs = 0;

static uint16_t read16(const uint8_t *buf) {
    return buf[0] | (buf[1] << 8);
}

static uint32_t read32(const uint8_t *buf) {
    return buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
}

static void fat16_read_sector(fat16_fs_t *fs, uint32_t lba, uint8_t *buffer) {
    fs->dev->read_sector(fs->dev, lba, buffer);
}

static void fat16_write_sector(fat16_fs_t *fs, uint32_t lba, const uint8_t *buffer) {
    fs->dev->write_sector(fs->dev, lba, buffer);
}

static void fat16_read_cluster(fat16_fs_t *fs, uint16_t cluster, uint8_t *buffer) {
    uint32_t lba = fs->data_start + (cluster - 2) * fs->sectors_per_cluster;
    for (uint8_t i = 0; i < fs->sectors_per_cluster; ++i) {
        fat16_read_sector(fs, lba + i, buffer + i * fs->bytes_per_sector);
    }
}

static uint16_t fat16_next_cluster(fat16_fs_t *fs, uint16_t cluster) {
    uint32_t fat_lba = fs->fat_start + (cluster * 2) / fs->bytes_per_sector;
    uint8_t sector[512];
    fat16_read_sector(fs, fat_lba, sector);
    uint16_t next = *(uint16_t *)(sector + (cluster * 2 % fs->bytes_per_sector));
    return next;
}

static void fat16_set_next_cluster(fat16_fs_t *fs, uint16_t cluster, uint16_t next) {
    uint32_t fat_lba = fs->fat_start + (cluster * 2) / fs->bytes_per_sector;
    uint8_t sector[8];
    fat16_read_sector(fs, fat_lba, sector);
    *(uint16_t *)(sector + (cluster * 2 % fs->bytes_per_sector)) = next;
    fat16_write_sector(fs, fat_lba, sector);
}

static uint16_t fat16_alloc_cluster(fat16_fs_t *fs) {
    uint8_t sector[512];
    for (uint32_t cl = 2; cl < fs->sectors_per_fat * fs->bytes_per_sector / 2; ++cl) {
        uint32_t fat_lba = fs->fat_start + (cl * 2) / fs->bytes_per_sector;
        fat16_read_sector(fs, fat_lba, sector);
        uint16_t entry = *(uint16_t *)(sector + (cl * 2 % fs->bytes_per_sector));
        if (entry == 0) {
            fat16_set_next_cluster(fs, cl, 0xFFF8);
            return cl;
        }
    }
    return 0;
}

static int fat16_find_free_dirent(fat16_fs_t *fs, uint32_t dir_start, uint32_t dir_sectors, uint32_t *out_sector, uint32_t *out_offset) {
    uint8_t sector[512];
    for (uint32_t i = 0; i < dir_sectors; ++i) {
        fat16_read_sector(fs, dir_start + i, sector);
        for (uint32_t j = 0; j < fs->bytes_per_sector; j += 32) {
            if (sector[j] == 0x00 || sector[j] == 0xE5) {
                *out_sector = dir_start + i;
                *out_offset = j;
                return 1;
            }
        }
    }
    return 0;
}

static int fat16_find_free_dirent_in_dir(fat16_fs_t *fs, uint16_t first_cluster, uint32_t *out_sector, uint32_t *out_offset) {
    if (first_cluster == 0) {
        return fat16_find_free_dirent(fs, fs->root_dir_start, fs->root_dir_sectors, out_sector, out_offset);
    }

    uint32_t cluster = first_cluster;
    uint32_t cluster_size = fs->sectors_per_cluster * fs->bytes_per_sector;
    uint8_t buf[512];

    (void)cluster_size;

    while (cluster < 0xFFF8) {
        uint32_t lba = fs->data_start + (cluster - 2) * fs->sectors_per_cluster;
        for (uint8_t i = 0; i < fs->sectors_per_cluster; ++i) {
            fat16_read_sector(fs, lba + i, buf);
            for (uint32_t j = 0; j < fs->bytes_per_sector; j += 32) {
                if (buf[j] == 0x00 || buf[j] == 0xE5) {
                    *out_sector = lba + i;
                    *out_offset = j;
                    return 1;
                }
            }
        }
        cluster = fat16_next_cluster(fs, cluster);
    }
    return 0;
}

static void fat16_name_to_str(char *out, const uint8_t *name) {
    int i = 0, j = 0;
    for (; i < 8 && name[i] != ' '; ++i) out[j++] = name[i];
    if (name[8] != ' ') {
        out[j++] = '.';
        for (i = 8; i < 11 && name[i] != ' '; ++i) out[j++] = name[i];
    }
    out[j] = 0;
}

static void str_to_fat16_name(const char *name, char *out) {
    memset(out, ' ', 11);
    int i = 0, j = 0;
    while (name[i] && name[i] != '.' && j < 8) {
        char c = name[i++];
        if (c >= 'a' && c <= 'z') c -= 32;
        out[j++] = c;
    }
    if (name[i] == '.') {
        i++;
        j = 8;
        while (name[i] && j < 11) {
            char c = name[i++];
            if (c >= 'a' && c <= 'z') c -= 32;
            out[j++] = c;
        }
    }
}

static void fat16_add_child(fs_node_t *parent, fs_node_t *child) {
    if (!parent || !child) return;

    if (!parent->children) {
        parent->children = malloc(sizeof(fs_node_t *) * 4);
        parent->child_capacity = 4;
    } else if (parent->child_count >= parent->child_capacity) {
        size_t new_cap = parent->child_capacity * 2;
        parent->children = realloc(parent->children, sizeof(fs_node_t *) * new_cap);
        parent->child_capacity = new_cap;
    }

    child->parent = parent;
    parent->children[parent->child_count++] = child;
}

static fs_node_t *fat16_find_child(fs_node_t *dir, const char *name) {
    if (!dir || dir->type != FS_DIR) return NULL;

    for (size_t i = 0; i < dir->child_count; ++i) {
        if (strcmp(dir->children[i]->name, name) == 0) {
            return dir->children[i];
        }
    }
    return NULL;
}

static void fat16_remove_child(fs_node_t *parent, fs_node_t *child) {
    if (!parent || !child) return;
    for (size_t i = 0; i < parent->child_count; ++i) {
        if (parent->children[i] == child) {
            for (size_t j = i; j < parent->child_count - 1; ++j) {
                parent->children[j] = parent->children[j + 1];
            }
            parent->child_count--;
            break;
        }
    }
}

static void fat16_scan_dir(fat16_fs_t *fs, fs_node_t *dir, uint16_t first_cluster, int is_root) {
    uint32_t start_lba, sector_count;

    if (is_root) {
        start_lba = fs->root_dir_start;
        sector_count = fs->root_dir_sectors;
    } else {
        start_lba = fs->data_start + (first_cluster - 2) * fs->sectors_per_cluster;
        sector_count = fs->sectors_per_cluster;
    }

    uint8_t sector[512];
    for (uint32_t i = 0; i < sector_count; ++i) {
        fat16_read_sector(fs, start_lba + i, sector);
        for (uint32_t j = 0; j < fs->bytes_per_sector / 32; ++j) {
            uint8_t *ent = sector + j * 32;
            if (ent[0] == 0x00 || ent[0] == 0xE5) continue;
            uint8_t attr = ent[11];

            if (attr & 0x08) continue;

            fs_node_t *node = malloc(sizeof(fs_node_t));
            memset(node, 0, sizeof(fs_node_t));

            char fname[13];
            fat16_name_to_str(fname, ent);
            strlcpy(node->name, fname, sizeof(node->name));

            if (attr & 0x10) {
                node->type = FS_DIR;
                fat16_file_t *f = malloc(sizeof(fat16_file_t));
                f->fs = fs;
                f->first_cluster = read16(ent + 26);
                f->size = 0;
                node->private = f;
                fat16_add_child(dir, node);

                if (strcmp(fname, ".") && strcmp(fname, "..")) {
                    uint16_t cluster = read16(ent + 26);
                    if (cluster != 0) {
                        fat16_scan_dir(fs, node, cluster, 0);
                    }
                }
            } else {
                node->type = FS_FILE;
                fat16_file_t *f = malloc(sizeof(fat16_file_t));
                f->fs = fs;
                f->first_cluster = read16(ent + 26);
                f->size = read32(ent + 28);
                node->private = f;
                node->size = f->size;

                fat16_add_child(dir, node);
            }
        }
    }
}

static size_t fat16_read_fs(fs_node_t *node, size_t offset, char *buffer, size_t len) {
    if (!node || !node->private) {
        return 0;
    }

    fat16_file_t *f = node->private;
    fat16_fs_t *fs = f->fs;

    uint32_t cluster_size = fs->sectors_per_cluster * fs->bytes_per_sector;
    uint16_t cluster = f->first_cluster;

    size_t cluster_skip = offset / cluster_size;
    size_t cluster_offset = offset % cluster_size;
    for (size_t i = 0; i < cluster_skip; ++i) {
        cluster = fat16_next_cluster(fs, cluster);
        if (cluster >= 0xFFF8) {
            return 0;
        }
    }

    size_t read_total = 0;
    uint8_t buf[cluster_size];
    while (len > 0 && cluster < 0xFFF8) {
        fat16_read_cluster(fs, cluster, buf);

        size_t to_read = cluster_size - cluster_offset;
        if (to_read > len) {
            to_read = len;
        }

        if (read_total + to_read > f->size - offset) {
            to_read = f->size - offset - read_total;
        }

        memcpy(buffer + read_total, buf + cluster_offset, to_read);
        read_total += to_read;
        len -= to_read;
        cluster_offset = 0;
        cluster = fat16_next_cluster(fs, cluster);
    }

    return read_total;
}

static size_t fat16_write_fs(fs_node_t *node, size_t offset, const char *buffer, size_t len) {
    if (!node || node->type != FS_FILE || !node->private) {
        return 0;
    }
    fat16_file_t *f = node->private;
    fat16_fs_t *fs = f->fs;

    uint32_t cluster_size = fs->sectors_per_cluster * fs->bytes_per_sector;
    uint16_t cluster = f->first_cluster;
    size_t cluster_skip = offset / cluster_size;
    size_t cluster_offset = offset & cluster_size;

    for (size_t i = 0; i < cluster_skip; ++i) {
        uint16_t next = fat16_next_cluster(fs, cluster);
        if (next >= 0xFFF8) {
            uint16_t new_cl = fat16_alloc_cluster(fs);
            if (!new_cl) {
                return 0;
            }
            fat16_set_next_cluster(fs, cluster, new_cl);
            cluster = new_cl;
        } else {
            cluster = next;
        }
    }

    size_t written = 0;
    uint8_t buf[cluster_size];

    while (len > 0) {
        fat16_read_cluster(fs, cluster, buf);

        size_t to_write = cluster_size - cluster_offset;
        if (to_write > len) {
            to_write = len;
        }
        memcpy(buf + cluster_offset, buffer + written, to_write);

        uint32_t lba = fs->data_start + (cluster - 2) * fs->sectors_per_cluster;
        for (uint8_t i = 0; i < fs->sectors_per_cluster; ++i) {
            fat16_write_sector(fs, lba + i, buf + i * fs->bytes_per_sector);
        }

        written += to_write;
        len -= to_write;
        cluster_offset = 0;

        if (len > 0) {
            uint16_t next = fat16_next_cluster(fs, cluster);
            if (next >= 0xFFF8) {
                uint16_t new_cl = fat16_alloc_cluster(fs);
                if (!new_cl) break;
                fat16_set_next_cluster(fs, cluster, new_cl);
                cluster = new_cl;
            } else {
                cluster = next;
            }
        }
    }

    if (offset + written > f->size) f->size = offset + written;
    node->size = f->size;

    return written;
}

static fs_node_t *fat16_resolve_from_fs(filesystem_t *fs, fs_node_t *start, const char *path) {
    if (!fs || !start || !path) {
        return NULL;
    }

    fs_node_t *current = path[0] == '/' ? fs->root : start;

    while (*path == '\0') {
        return current;
    }

    char temp[MAX_PATH_LEN];
    strlcpy(temp, path, sizeof(temp));
    char *temp_ptr = temp;
    char *token = strktok(&temp_ptr, "/");

    while (token) {
        if (strcmp(token, ".") == 0) {
            token = strktok(&temp_ptr, "/");
            continue;
        }

        if (strcmp(token, "..") == 0) {
            if (current->parent) current = current->parent;
            token = strktok(&temp_ptr, "/");
            continue;
        }

        if (current->type != FS_DIR) {
            return NULL;
        }

        fs_node_t *next = fat16_find_child(current, token);
        if (!next) {
            return NULL;
        }

        current = next;
        token = strktok(&temp_ptr, "/");
    }

    return current;
}

static fs_node_t *fat16_resolve_fs(filesystem_t *fs, const char *path) {
    return fat16_resolve_from_fs(fs, fs->root, path);
}

static fs_node_t *fat16_create(struct filesystem *fsys, fs_node_t *parent, const char *name, fs_node_type_t type) {
    if (!fsys || !parent || parent->type != FS_DIR) return NULL;

    fat16_file_t *parent_f = parent->private;
    if (!parent_f) return NULL;

    fat16_fs_t *fs = parent_f->fs;
    uint32_t sector = 0, offset = 0;

    if (!fat16_find_free_dirent_in_dir(fs, parent_f->first_cluster, &sector, &offset)) {
        return NULL;
    }

    uint8_t buf[512];
    fat16_read_sector(fs, sector, buf);

    uint8_t *ent = buf + offset;
    memset(ent, 0, 32);

    char fat_name[11];
    str_to_fat16_name(name, fat_name);
    memcpy(ent, fat_name, 11);

    ent[11] = (type == FS_DIR) ? 0x10 : 0x00;

    fat16_write_sector(fs, sector, buf);

    fs_node_t *node = malloc(sizeof(fs_node_t));
    memset(node, 0, sizeof(fs_node_t));
    strlcpy(node->name, name, sizeof(node->name));
    node->type = type;

    fat16_file_t *f = malloc(sizeof(fat16_file_t));
    f->fs = fs;
    f->first_cluster = 0;
    f->size = 0;
    node->private = f;

    fat16_add_child(parent, node);
    return node;
}

static int fat16_remove(struct filesystem *fsys, fs_node_t *node, int recursive) {
    if (!fsys || !node || !node->parent) return -1;

    (void)recursive;

    fat16_file_t *node_f = node->private;
    fat16_file_t *parent_f = node->parent->private;
    if (!node_f || !parent_f) return -1;

    fat16_fs_t *fs = parent_f->fs;
    uint32_t cluster = parent_f->first_cluster;
    uint32_t start_lba, sector_count;
    int is_root = (cluster == 0);

    if (is_root) {
        start_lba = fs->root_dir_start;
        sector_count = fs->root_dir_sectors;
    } else {
        start_lba = fs->data_start + (cluster - 2) * fs->sectors_per_cluster;
        sector_count = fs->sectors_per_cluster;
    }

    char fat_name[11];
    str_to_fat16_name(node->name, fat_name);

    uint8_t buf[512];
    int found = 0;

    while (!found && cluster < 0xFFF8) {
        for (uint32_t i = 0; i < sector_count; ++i) {
            fat16_read_sector(fs, start_lba + i, buf);
            for (uint32_t j = 0; j < fs->bytes_per_sector; j += 32) {
                uint8_t *ent = buf + j;
                if (ent[0] == 0x00 || ent[0] == 0xE5) continue;

                if (memcmp(ent, fat_name, 11) == 0) {
                    ent[0] = 0xE5;
                    fat16_write_sector(fs, start_lba + i, buf);
                    found = 1;
                    break;
                }
            }
            if (found) break;
        }
        if (found) break;

        if (is_root) break;

        cluster = fat16_next_cluster(fs, cluster);
        if (cluster < 0xFFF8) {
            start_lba = fs->data_start + (cluster - 2) * fs->sectors_per_cluster;
        }
    }

    if (!found) return -1;

    uint16_t c = node_f->first_cluster;
    while (c != 0 && c < 0xFFF8) {
        uint16_t next = fat16_next_cluster(fs, c);
        fat16_set_next_cluster(fs, c, 0);
        c = next;
    }

    fat16_remove_child(node->parent, node);
    free(node_f);
    free(node);

    return 0;
}

void fat16_init(block_device_t *dev) {
    uint8_t sector[512];
    dev->read_sector(dev, 0, sector);

    fat16_fs_t *fs = malloc(sizeof(fat16_fs_t));
    fs->dev = dev;
    fs->bytes_per_sector = read16(sector + 11);
    fs->sectors_per_cluster = sector[13];
    fs->reserved_sectors = read16(sector + 14);
    fs->num_fats = sector[16];
    fs->root_entries = read16(sector + 17);
    fs->sectors_per_fat = read16(sector + 22);
    fs->fat_start = fs->reserved_sectors;
    fs->root_dir_start = fs->fat_start + fs->num_fats * fs->sectors_per_fat;
    fs->root_dir_sectors = (fs->root_entries * 32 + fs->bytes_per_sector - 1) / fs->bytes_per_sector;
    fs->data_start = fs->root_dir_start + fs->root_dir_sectors;

    fat16_current_fs = fs;

    fat16_fs.root = malloc(sizeof(fs_node_t));
    memset(fat16_fs.root, 0, sizeof(fs_node_t));
    strlcpy(fat16_fs.root->name, "/", sizeof(fat16_fs.root->name));
    fat16_fs.root->type = FS_DIR;

    fat16_file_t *root_f = malloc(sizeof(fat16_file_t));
    root_f->fs = fs;
    root_f->first_cluster = 0;
    root_f->size = 0;
    fat16_fs.root->private = root_f;

    fat16_scan_dir(fs, fat16_fs.root, 0, 1);

    fat16_fs.resolve = fat16_resolve_fs;
    fat16_fs.resolve_from = fat16_resolve_from_fs;
    fat16_fs.read = fat16_read_fs;
    fat16_fs.write = fat16_write_fs;
    fat16_fs.create = fat16_create;
    fat16_fs.remove = fat16_remove;

    vfs_mount_root(&fat16_fs);
}