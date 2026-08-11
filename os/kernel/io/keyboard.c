#include <arch.h>
#include <lib/stdint.h>
#include <io/kernel_io.h>
#include <shell/input.h>
#include <timer/timer.h>

#include <lib/string.h>
#include "keyboard.h"

typedef struct {
    uint8_t pressed;
    uint64_t next_repeat;
} key_state_t;

uint8_t ctrl_pressed  = 0;
uint8_t shift_pressed = 0;
static key_state_t keys[128];

static key_event_t key_queue[KEY_QUEUE_SIZE];
static uint32_t queue_head = 0;
static uint32_t queue_tail = 0;

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

char keycode_to_char(uint8_t scancode, uint8_t shift) {
    if (scancode >= 128) return 0;
    return shift ? scancode_table_shift[scancode] : scancode_table[scancode];
}

void handle_key_release(uint8_t released, uint8_t *shift) {
    if (released == 0x2A || released == 0x36) *shift = 0;
}

static void queue_push_event(uint8_t scancode, uint8_t pressed) {
    uint32_t next = (queue_head + 1) % KEY_QUEUE_SIZE;
    if (next != queue_tail) {
        key_queue[queue_head].scancode = scancode;
        key_queue[queue_head].pressed  = pressed;
        queue_head = next;
    }
}

int keyboard_pop_event(key_event_t *event) {
    if (queue_head == queue_tail) {
        return 0; // empty queue
    }
    *event = key_queue[queue_tail];
    queue_tail = (queue_tail + 1) % KEY_QUEUE_SIZE;
    return 1;
}

void keyboard_callback(uint8_t sc) {
    uint8_t pressed = !(sc & 0x80);
    uint8_t scancode = sc & 0x7F;

    if (scancode == 0x1D) {
        ctrl_pressed = pressed;
    }
    
    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = pressed;
    }

    // Ignore typematic repeat flags
    if (pressed && keys[scancode].pressed) {
        return;
    }

    keys[scancode].pressed = pressed;
    if (pressed) {
        keys[scancode].next_repeat = timer_ms() + 400;
    }

    if (pressed) {
        if (scancode == 0x48 && !shift_pressed) {
            scroll_up();
            return;
        } else if (scancode == 0x50) {
            scroll_down();
            return;
        }
    }

    queue_push_event(scancode, pressed);
}

void keyboard_timer_tick(void) {
    uint64_t now = timer_ms();

    for (int i = 0; i < 128; ++i) {
        if (!keys[i].pressed) {
            continue;
        }

        if (now >= keys[i].next_repeat) {
            queue_push_event(i, 1);
            keys[i].next_repeat += 40;
        }
    }
}