#include <arch.h>
#include <block/block.h>
#include <lib/stddef.h>
#include <lib/stdint.h>

#define ATA_IO   0x1F0
#define ATA_CTRL 0x3F6

#define ATA_REG_DATA      0x00
#define ATA_REG_SECCOUNT0 0x02
#define ATA_REG_LBA0      0x03
#define ATA_REG_LBA1      0x04
#define ATA_REG_LBA2      0x05
#define ATA_REG_HDDEVSEL  0x06
#define ATA_REG_COMMAND   0x07
#define ATA_REG_STATUS    0x07

#define ATA_CMD_READ_SECTORS  0x20
#define ATA_CMD_WRITE_SECTORS 0x30

#define ATA_SR_BSY 0x80
#define ATA_SR_DRQ 0x08

static int ata_read_sector(block_device_t *dev, uint32_t lba, uint8_t *buffer);
static int ata_write_sector(block_device_t *dev, uint32_t lba, const uint8_t *buffer);

static block_device_t ata_device = {
    .read_sector = ata_read_sector,
    .write_sector = ata_write_sector,
    .sector_size = 512,
    .driver_data = 0
};

static void ata_wait_busy(void) {
    while (inb(ATA_IO + ATA_REG_STATUS) & ATA_SR_BSY);
}

static void ata_wait_drq(void) {
    while (!(inb(ATA_IO + ATA_REG_STATUS) & ATA_SR_DRQ));
}

static int ata_read_sector(block_device_t *dev, uint32_t lba, uint8_t *buffer) {
    (void)dev;

    ata_wait_busy();

    outb(ATA_IO + ATA_REG_HDDEVSEL, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_IO + ATA_REG_SECCOUNT0, 1);
    outb(ATA_IO + ATA_REG_LBA0, (uint8_t)lba);
    outb(ATA_IO + ATA_REG_LBA1, (uint8_t)(lba >> 8));
    outb(ATA_IO + ATA_REG_LBA2, (uint8_t)(lba >> 16));
    outb(ATA_IO + ATA_REG_COMMAND, ATA_CMD_READ_SECTORS);

    ata_wait_busy();
    ata_wait_drq();

    for (size_t i = 0; i < 256; ++i) {
        uint16_t data = inw(ATA_IO);
        buffer[i * 2] = data & 0xFF;
        buffer[i * 2 + 1] = data >> 8;
    }

    return 0;
}

static int ata_write_sector(block_device_t *dev, uint32_t lba, const uint8_t *buffer) {
    (void)dev;

    ata_wait_busy();

    outb(ATA_IO + ATA_REG_HDDEVSEL, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_IO + ATA_REG_SECCOUNT0, 1);
    outb(ATA_IO + ATA_REG_LBA0, (uint8_t)lba);
    outb(ATA_IO + ATA_REG_LBA1, (uint8_t)(lba >> 8));
    outb(ATA_IO + ATA_REG_LBA2, (uint8_t)(lba >> 16));
    outb(ATA_IO + ATA_REG_COMMAND, ATA_CMD_WRITE_SECTORS);

    ata_wait_busy();
    ata_wait_drq();

    for (size_t i = 0; i < 256; ++i) {
        uint16_t data = buffer[i * 2] | (buffer[i * 2 + 1] << 8);
        outw(ATA_IO, data);
    }

    ata_wait_busy();
    return 0;
}

void ata_init(void) {
    block_register_device(&ata_device);
}