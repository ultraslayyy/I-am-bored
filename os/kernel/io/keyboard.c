#include <stdint.h>
#include <arch.h>
#include <shell/input.h>
#include <io/kernel_io.h>

static uint8_t shift_pressed = 0;
static uint8_t key_pressed[128] = {0};

char scancode_table[128] = {
    0,27,'1','2','3','4','5','6','7','8','9','0','-','=','\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',0,
    'a','s','d','f','g','h','j','k','l',';','\'','`',0,'\\','z','x',
    'c','v','b','n','m',',','.','/',0,'*',0,' ',0
};

char scancode_table_shift[128] = {
    0,27,'!','@','#','$','%','^','&','*','(',')','_','+','\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',0,
    'A','S','D','F','G','H','J','K','L',':','"','~',0,'|','Z','X',
    'C','V','B','N','M','<','>','?',0,'*',0,' ',0
};

char keycode_to_char(uint8_t scancode, uint8_t shift_pressed) {
    if (scancode >= 128) return 0;
    return shift_pressed ? scancode_table_shift[scancode] : scancode_table[scancode];
}

void handle_key_release(uint8_t released, uint8_t *shift) {
    if (released == 0x2A || released == 0x36) *shift = 0;
}

void keyboard_callback(void) {
    uint8_t sc = inb(0x60);
    
    if (sc & 0x80) {
        uint8_t released = sc & 0x7F;
        key_pressed[released] = 0;
        handle_key_release(released, &shift_pressed);
        return;
    }

    if (key_pressed[sc]) return;
    key_pressed[sc] = 1;

    if (sc == 0x2A || sc == 0x36) {
        shift_pressed = 1;
        return;
    }
    
    if (sc == 0x48) {
        scroll_up();
        return;
    } else if (sc == 0x50) {
        scroll_down();
        return;
    }

    char c = keycode_to_char(sc, shift_pressed);
    if (c) {
        shell_handle_char(c);
    }
}
