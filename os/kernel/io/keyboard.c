#include <stdint.h>

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

void handle_key_release(uint8_t released, uint8_t *shift_pressed) {
    if (released == 0x2A || released == 0x36) *shift_pressed = 0;
}
