#include <lib/stdint.h>

#define PIT_BASE_FREQ 1193180
#define PIT_INIT_FREQ 100

void pit_init(uint32_t frequency);