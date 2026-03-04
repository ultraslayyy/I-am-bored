#pragma once
#include <lib/stdint.h>

typedef struct block_device {
    int (*read_sector)(struct block_device *dev, uint32_t lba, uint8_t *buffer);
    int (*write_sector)(struct block_device *dev, uint32_t lba, const uint8_t *buffer);
    uint32_t sector_size;
    void *driver_data;
} block_device_t;

void block_register_device(block_device_t *dev);
block_device_t *block_get_device(void);