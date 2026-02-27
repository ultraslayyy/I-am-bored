#pragma once
#include <lib/stdint.h>

void tss_init(uint32_t kernel_stack);
void tss_set_stack(uint32_t stack);