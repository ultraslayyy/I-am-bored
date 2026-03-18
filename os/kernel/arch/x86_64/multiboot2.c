#include <core/boot_info.h>
#include <lib/stdint.h>

#define MB2_TAG_MMAP 6
#define MB2_TAG_FRAMEBUFFER 8

typedef struct {
    uint32_t type;
    uint32_t size;
} mb2_tag_t;

typedef struct {
    mb2_tag_t tag;
    uint32_t entry_size;
    uint32_t entry_version;
} mb2_mmap_tag_t;

typedef struct {
    uint64_t addr;
    uint64_t len;
    uint32_t type;
    uint32_t zero;
} __attribute__((packed)) mb2_mmap_entry_t;

typedef struct {
    mb2_tag_t tag;
    uint64_t addr;
    uint32_t pitch;
    uint32_t width;
    uint32_t height;
    uint8_t bpp;
    uint8_t type;
    uint8_t reserved;
} mb2_framebuffer_tag_t;

void multiboot2_parse(void *mb_info, boot_info_t *out) {
    uint8_t *ptr = (uint8_t *)mb_info + 8;

    out->flags = *(uint32_t *)mb_info;

    out->mmap_entries = 0;

    while (1) {
        mb2_tag_t *tag = (mb2_tag_t *)ptr;
        if (tag->type == 0) break;

        if (tag->type == MB2_TAG_MMAP) {
            mb2_mmap_tag_t *mmap = (mb2_mmap_tag_t *)tag;
            mb2_mmap_entry_t *e = (mb2_mmap_entry_t *)(mmap + 1);

            uint32_t count = (mmap->tag.size - sizeof(*mmap)) / mmap->entry_size;

            for (uint32_t i = 0; i < count && out->mmap_entries < 128; ++i) {
                out->mmap[out->mmap_entries].base   = e[i].addr;
                out->mmap[out->mmap_entries].length = e[i].len;
                out->mmap[out->mmap_entries].type   = e[i].type;
                out->mmap_entries++;
            }
        } else if (tag->type == MB2_TAG_FRAMEBUFFER) {
            mb2_framebuffer_tag_t *fb = (mb2_framebuffer_tag_t *)tag;
            out->framebuffer_addr = fb->addr;
            out->framebuffer_pitch = fb->pitch;
            out->framebuffer_width = fb->width;
            out->framebuffer_height = fb->height;
            out->framebuffer_bpp = fb->bpp;
            out->framebuffer_type = fb->type;
        }

        ptr += (tag->size + 7) & ~7;
    }
}