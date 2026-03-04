#include "block.h"

static block_device_t *primary_device = 0;

void block_register_device(block_device_t *dev) {
    primary_device = dev;
}

block_device_t *block_get_device(void) {
    return primary_device;
}