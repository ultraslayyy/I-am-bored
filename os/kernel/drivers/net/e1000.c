#include <arch.h>
#include <drivers/pci/pci.h>
#include <io/kernel_io.h>
#include <lib/stddef.h>
#include <lib/stdint.h>
#include <lib/stdlib.h>
#include <lib/string.h>
#include <net/netdev.h>
#include "e1000.h"

#define E1000_VENDOR_ID 0x8086
#define E1000_DEVICE_ID 0x100E

#define E1000_MMIO_PAGES 32

#define E1000_REG_CTRL   0x0000
#define E1000_REG_STATUS 0x0008
#define E1000_REG_EERD   0x0014
#define E1000_REG_IMS    0x00D0
#define E1000_REG_RCTL   0x0100
#define E1000_REG_TCTL   0x0400
#define E1000_REG_RDBAL  0x2800
#define E1000_REG_RDLEN  0x2808
#define E1000_REG_RDH    0x2810
#define E1000_REG_RDT    0x2818
#define E1000_REG_TDBAL  0x3800
#define E1000_REG_TDLEN  0x3808
#define E1000_REG_TDH    0x3810
#define E1000_REG_TDT    0x3818

#define E1000_NUM_RX_DESC 32
#define E1000_NUM_TX_DESC 32

typedef struct {
    uint64_t addr;
    uint16_t length;
    uint16_t checksum;
    uint8_t  status;
    uint8_t  errors;
    uint16_t special;
} __attribute__((packed)) e1000_rx_desc_t;

typedef struct {
    uint64_t addr;
    uint16_t length;
    uint8_t  cso;
    uint8_t  cmd;
    uint8_t  status;
    uint8_t  css;
    uint16_t special;
} __attribute__((packed)) e1000_tx_desc_t;

static volatile uint32_t *e1000_mmio;

static e1000_rx_desc_t rx_descs[E1000_NUM_RX_DESC] __attribute__((aligned(16)));
static e1000_tx_desc_t tx_descs[E1000_NUM_TX_DESC] __attribute__((aligned(16)));

static uint8_t rx_buffers[E1000_NUM_RX_DESC][2048];
static uint8_t tx_buffers[E1000_NUM_TX_DESC][2048];

static uint32_t rx_tail = 0;
static uint32_t tx_tail = 0;

static uint8_t mac_addr[6];

static netdev_t e1000_dev;

static inline void e1000_write(uint32_t reg, uint32_t val) {
    e1000_mmio[reg / 4] = val;
}

static inline uint32_t e1000_read(uint32_t reg) {
    return e1000_mmio[reg / 4];
}

void e1000_send_frame(uint8_t *data, size_t len) {
    if (len > 2048) return;

    e1000_tx_desc_t *desc = &tx_descs[tx_tail];

    while (!(desc->status & 0x1));

    memcpy((void *)(uintptr_t)desc->addr, data, len);

    desc->length = len;
    desc->cmd = (1 << 0) | (1 << 1);
    desc->status = 0;

    uint32_t old_tail = tx_tail;
    tx_tail = (tx_tail + 1) % E1000_NUM_TX_DESC;

    put_string("e1000_send_frame\n", DEFAULT_ATTR);
    e1000_write(E1000_REG_TDT, tx_tail);
}

int e1000_recv_frame(uint8_t *buffer, size_t max_len) {
    e1000_rx_desc_t *desc = &rx_descs[rx_tail];

    if (!(desc->status & 0x01)) {
        return 0;
    }

    size_t len = desc->length;
    if (len > max_len) {
        len = max_len;
    }

    memcpy(buffer, (void *)(uintptr_t)desc->addr, len);

    desc->status = 0;
    e1000_write(E1000_REG_RDT, rx_tail);
    rx_tail = (rx_tail + 1) % E1000_NUM_RX_DESC;

    return (int)len;
}

static int e1000_net_send(netdev_t *dev, const void *data, size_t len) {
    e1000_send_frame((uint8_t *)data, len);
    return (int)len;
}

static int e1000_net_recv(netdev_t *dev, void *buf, size_t max_len) {
    return e1000_recv_frame(buf, max_len);
}

void e1000_init(void) {
    uint8_t bus, slot, func;

    if (!pci_find_device(E1000_VENDOR_ID, E1000_DEVICE_ID, &bus, &slot, &func)) {
        put_string("e1000: not found\n", DEFAULT_ATTR);
        return;
    }

    uint16_t cmd = pci_read_word(bus, slot, func, 0x04);
    cmd |= (1 << 2);
    cmd |= (1 << 1);
    pci_write_word(bus, slot, func, 0x04, cmd);

    uint32_t bar0 = pci_read_dword(bus, slot, func, 0x10);
    uint32_t mmio_base = bar0 & 0xFFFFFFF0;
    e1000_mmio = (volatile uint32_t *)mmio_base;

    char text[128];
    snprintf(text, sizeof(text), "e1000 MMIO @ %x\n", mmio_base);
    put_string(text, DEFAULT_ATTR);

    for (uint32_t i = 0; i < E1000_MMIO_PAGES; ++i) {
        map_page(mmio_base + i * 0x1000, mmio_base + i * 0x1000, PAGE_PRESENT | PAGE_RW);
    }

    e1000_write(E1000_REG_CTRL, e1000_read(E1000_REG_CTRL) | (1 << 26));
    for (volatile int i = 0; i < 100000; ++i);

    uint32_t mac_low  = e1000_read(0x5400);
    uint32_t mac_high = e1000_read(0x5404);

    mac_addr[0] = mac_low & 0xFF;
    mac_addr[1] = (mac_low >> 8) & 0xFF;
    mac_addr[2] = (mac_low >> 16) & 0xFF;
    mac_addr[3] = (mac_low >> 24) & 0xFF;
    mac_addr[4] = mac_high & 0xFF;
    mac_addr[5] = (mac_high >> 8) & 0xFF;

    char buf[128];
    snprintf(buf, sizeof(buf),
        "e1000 MAC %x:%x:%x:%x:%x:%x\n",
        mac_addr[0], mac_addr[1], mac_addr[2],
        mac_addr[3], mac_addr[4], mac_addr[5]);
    
    put_string(buf, DEFAULT_ATTR);

    for (int i = 0; i < E1000_NUM_RX_DESC; ++i) {
        uint32_t buf_phys = (uint32_t)(uintptr_t)rx_buffers[i] & 0xFFFFF000;
        map_page(buf_phys, buf_phys, PAGE_PRESENT | PAGE_RW);
        rx_descs[i].addr = (uint64_t)(uintptr_t)rx_buffers[i];
        rx_descs[i].status = 0;
    }

    e1000_write(E1000_REG_RDBAL, (uint32_t)(uintptr_t)rx_descs);
    e1000_write(E1000_REG_RDLEN, E1000_NUM_RX_DESC * sizeof(e1000_rx_desc_t));
    e1000_write(E1000_REG_RDH, 0);
    e1000_write(E1000_REG_RDT, E1000_NUM_RX_DESC - 1);

    e1000_write(E1000_REG_RCTL, (1 << 1) | (1 << 2) | (1 << 15) | (1 << 26));

    for (int i = 0; i < E1000_NUM_TX_DESC; ++i) {
        uint32_t buf_phys = (uint32_t)(uintptr_t)tx_buffers[i] & 0xFFFFF000;
        map_page(buf_phys, buf_phys, PAGE_PRESENT | PAGE_RW);
        tx_descs[i].addr = (uint64_t)(uintptr_t)tx_buffers[i];
        tx_descs[i].status = 0x1; // Done
    }

    e1000_write(E1000_REG_TDBAL, (uint32_t)(uintptr_t)tx_descs);
    e1000_write(E1000_REG_TDLEN, E1000_NUM_TX_DESC * sizeof(e1000_tx_desc_t));
    e1000_write(E1000_REG_TDH, 0);
    e1000_write(E1000_REG_TDT, 0);

    e1000_write(E1000_REG_TCTL, (1 << 1) | (1 << 3));

    memset(&e1000_dev, 0, sizeof(netdev_t));
    strlcpy(e1000_dev.name, "eth0", sizeof(e1000_dev.name));
    memcpy(e1000_dev.mac, mac_addr, 6);

    e1000_dev.send = e1000_net_send;
    e1000_dev.recv = e1000_net_recv;
    e1000_dev.priv = 0;

    netdev_register(&e1000_dev);

    put_string("e1000: initialised as eth0\n", DEFAULT_ATTR);
}