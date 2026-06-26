#include <drivers/video/font.h>
#include <drivers/video/vesa.h>
#include <lib/string.h>
#include "kernel_io.h"

uint32_t cursor_x = 0;
uint32_t cursor_y = 0;
uint32_t text_fg = 0xFFFFFF;
uint32_t text_bg = 0x000000;

#define MAX_TERM_LINES 1000
#define MAX_TERM_COLS 256

struct char_info {
    char c;
    uint32_t fg;
    uint32_t bg;
};

static struct char_info text_buffer[MAX_TERM_LINES][MAX_TERM_COLS];
static uint32_t term_line = 0;
static uint32_t term_col = 0;
static uint32_t view_line = 0;
static int is_scrolled_up = 0;

void update_screen(void) {
    uint32_t lines_per_screen = vesa_height / FONT_HEIGHT;
    uint32_t cols_per_line = vesa_width / FONT_WIDTH;

    vesa_clear_screen(text_bg);

    for (uint32_t r = 0; r < lines_per_screen; ++r) {
        uint32_t buf_line = view_line + r;
        if (buf_line > term_line) break;

        uint32_t idx = buf_line % MAX_TERM_LINES;
        for (uint32_t c = 0; c < cols_per_line; ++c) {
            char ch = text_buffer[idx][c].c;
            if (ch == 0) continue;
            vesa_draw_char(ch, c * FONT_WIDTH, r * FONT_HEIGHT, text_buffer[idx][c].fg, text_buffer[idx][c].bg);
        }
    }
}

void put_char(char c, uint8_t attr) {
    (void)attr;
    uint32_t cols_per_line = vesa_width / FONT_WIDTH;
    uint32_t lines_per_screen = vesa_height / FONT_HEIGHT;
    
    if (is_scrolled_up) {
        is_scrolled_up = 0;
        view_line = (term_line >= lines_per_screen) ? (term_line - lines_per_screen + 1) : 0;
        update_screen();
    }

    if (c == '\n') {
        term_col = 0;
        term_line++;
        
        uint32_t idx = term_line % MAX_TERM_LINES;
        for (uint32_t i = 0; i < MAX_TERM_COLS; ++i) {
            text_buffer[idx][i].c = 0;
        }
    } else if (c == '\r') {
        term_col = 0;
        return;
    } else if (c == '\b') {
        if (term_col > 0) {
            term_col--;
            text_buffer[term_line % MAX_TERM_LINES][term_col].c = 0;
            
            cursor_y = (term_line - view_line) * FONT_HEIGHT;
            cursor_x = term_col * FONT_WIDTH;
            vesa_draw_rect(cursor_x, cursor_y, FONT_WIDTH, FONT_HEIGHT, text_bg);
        }
        return;
    } else {
        if (term_col < cols_per_line) {
            uint32_t idx = term_line % MAX_TERM_LINES;
            text_buffer[idx][term_col].c = c;
            text_buffer[idx][term_col].fg = text_fg;
            text_buffer[idx][term_col].bg = text_bg;
            
            cursor_y = (term_line - view_line) * FONT_HEIGHT;
            cursor_x = term_col * FONT_WIDTH;
            
            vesa_draw_char(c, cursor_x, cursor_y, text_fg, text_bg);
            
            term_col++;
        }
        
        if (term_col >= cols_per_line) {
            term_col = 0;
            term_line++;
            
            uint32_t idx = term_line % MAX_TERM_LINES;
            for (uint32_t i = 0; i < MAX_TERM_COLS; ++i) {
                text_buffer[idx][i].c = 0;
            }
        }
    }

    if (term_line >= view_line + lines_per_screen) {
        view_line = term_line - lines_per_screen + 1;
        
        vesa_scroll(1, text_bg);
        
        uint32_t r = lines_per_screen - 1;
        uint32_t idx = term_line % MAX_TERM_LINES;
        for (uint32_t c = 0; c < cols_per_line; ++c) {
            char ch = text_buffer[idx][c].c;
            if (ch == 0) continue;
            vesa_draw_char(ch, c * FONT_WIDTH, r * FONT_HEIGHT, text_buffer[idx][c].fg, text_buffer[idx][c].bg);
        }
    }
}

void put_string(const char *str, uint8_t attr) {
    for (size_t i = 0; str[i]; ++i) {
        put_char(str[i], attr);
    }
}

void clear_screen(void) {
    vesa_clear_screen(text_bg);
    term_line = 0;
    term_col = 0;
    view_line = 0;
    is_scrolled_up = 0;
    
    memset(text_buffer, 0, sizeof(text_buffer));
}

void scroll_up(void) {
    if (view_line > 0) {
        uint32_t max_history = (term_line >= MAX_TERM_LINES) ? (term_line - MAX_TERM_LINES + 1) : 0;
        
        if (view_line > max_history) {
            view_line--;
            is_scrolled_up = 1;
            update_screen();
        }
    }
}

void scroll_down(void) {
    uint32_t lines_per_screen = vesa_height / FONT_HEIGHT;
    uint32_t max_view_line = (term_line >= lines_per_screen) ? (term_line - lines_per_screen + 1) : 0;
    
    if (view_line < max_view_line) {
        view_line++;
        if (view_line == max_view_line) is_scrolled_up = 0;
        update_screen();
    }
}
