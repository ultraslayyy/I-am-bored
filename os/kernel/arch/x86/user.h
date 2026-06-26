#pragma once
#include <lib/stdint.h>

void enter_user_mode(void (*entry)());
#define USER_STACK 0x500000
#define USER_CS    0x1B
#define USER_DS    0x23