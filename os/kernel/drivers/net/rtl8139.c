#include <arch.h>
#include <drivers/pci/pci.h>
#include <io/kernel_io.h>
#include <lib/string.h>
#include <net/netdev.h>

#define RTL_VENDOR_ID 0x10EC
#define RTL_DEVICE_ID 0x8139

#define RTL_RX_BUFFER_SIZE 8192
#define RTL_TX_SLOTS 4

#define RTL_REG_IDR0        0x00
#define RTL_REG_TXSTATUS0   0x10
#define RTL_REG_TXADDR0     0x20
#define RTL_REG_RXBUF       0x30
#define RTL_REG_COMMAND     0x37
#define RTL_REG_CAPR        0x38
#define RTL_REG_IMR         0x3C
#define RTL_REG_ISR         0x3E
#define RTL_REG_RXCONFIG    0x44
#define RTL_REG_TXCONFIG    0x40

#define RTL_CMD_RX_ENABLE 0x08
#define RTL_CMD_TX_ENABLE 0x04
#define RTL_CMD_RESET     0x10

static uint16_t rtl_io;

static uint8_t mac_addr[6];

static uint8_t rtl_rx_buffer[RTL_RX_BUFFER_SIZE + 1536];
static uint32_t rtl_rx_offset = 0;

static uint8_t rtl_tx_buffers[RTL_TX_SLOTS][2048];
static uint32_t rtl_tx_cur = 0;

static netdev_t rtl_dev;

static int rtl8139_send(netdev_t *dev, const void *data, size_t len) {
    (void)dev;
    if (len > 2048) {
        return -1;
    }

    uint32_t tx_len = len;
    if (tx_len < 60) {
        tx_len = 60;
    }

    memset(rtl_tx_buffers[rtl_tx_cur], 0, tx_len);
    memcpy(rtl_tx_buffers[rtl_tx_cur], data, len);

    outl(rtl_io + RTL_REG_TXADDR0 + rtl_tx_cur * 4, (uint32_t)(uintptr_t)rtl_tx_buffers[rtl_tx_cur]);
    outl(rtl_io + RTL_REG_TXSTATUS0 + rtl_tx_cur * 4, tx_len);
    rtl_tx_cur = (rtl_tx_cur + 1) % RTL_TX_SLOTS;

    return (int)len;
}

static int rtl8139_recv(netdev_t *dev, void *buf, size_t max_len) {
    (void)dev;
    
    if (inb(rtl_io + RTL_REG_COMMAND) & 0x01) {
        return 0;
    }

    uint16_t *hdr = (uint16_t *)(rtl_rx_buffer + rtl_rx_offset);

    uint16_t status = hdr[0];
    uint16_t len    = hdr[1];

    int ret = 0;

    uint32_t next_offset = (rtl_rx_offset + len + 4 + 3) & ~3;
    if (next_offset >= RTL_RX_BUFFER_SIZE) {
        next_offset -= RTL_RX_BUFFER_SIZE;
    }

    if (status & 1) {
        uint32_t pkt_len = len - 4;
        if (pkt_len > max_len) {
            pkt_len = max_len;
        }

        uint8_t *pkt = (uint8_t *)(hdr + 2);
        if (buf != NULL) {
            memcpy(buf, pkt, pkt_len);
        }
        ret = pkt_len;
    }

    rtl_rx_offset = next_offset;
    outw(rtl_io + RTL_REG_CAPR, rtl_rx_offset - 16);

    return ret;
}

void rtl8139_init(void) {
    uint8_t bus, slot, func;

    if (!pci_find_device(RTL_VENDOR_ID, RTL_DEVICE_ID, &bus, &slot, &func)) {
        put_string("rtl8139: not found\n", DEFAULT_ATTR);
        return;
    }

    uint16_t cmd = pci_read_word(bus, slot, func, 0x04);
    cmd |= (1 << 2) | (1 << 0);
    pci_write_word(bus, slot, func, 0x04, cmd);

    uint32_t bar0 = pci_read_dword(bus, slot, func, 0x10);
    rtl_io = bar0 & ~0x3;

    char text[128];
    snprintf(text, sizeof(text), "rtl8139 IO @ %x\n", rtl_io);
    put_string(text, DEFAULT_ATTR);

    outb(rtl_io + RTL_REG_COMMAND, RTL_CMD_RESET);

    while (inb(rtl_io + RTL_REG_COMMAND) & RTL_CMD_RESET);

    for (int i = 0; i < 6; i++) {
        mac_addr[i] = inb(rtl_io + RTL_REG_IDR0 + i);
    }

    char macbuf[128];
    snprintf(macbuf, sizeof(macbuf),
        "rtl8139 MAC %x:%x:%x:%x:%x:%x\n",
        mac_addr[0], mac_addr[1], mac_addr[2],
        mac_addr[3], mac_addr[4], mac_addr[5]);

    put_string(macbuf, DEFAULT_ATTR);

    outl(rtl_io + RTL_REG_RXBUF, (uint32_t)(uintptr_t)rtl_rx_buffer);

    outl(rtl_io + RTL_REG_RXCONFIG,
         (1 << 7) |   /* wrap */
         (1 << 3) |   /* broadcast */
         (1 << 2) |   /* multicast */
         (1 << 1) |   /* physical match */
         (1 << 0));   /* all packets */

    outb(rtl_io + RTL_REG_COMMAND, RTL_CMD_RX_ENABLE | RTL_CMD_TX_ENABLE);

    memset(&rtl_dev, 0, sizeof(netdev_t));

    strlcpy(rtl_dev.name, "eth1", sizeof(rtl_dev.name));
    memcpy(rtl_dev.mac, mac_addr, 6);

    rtl_dev.send = rtl8139_send;
    rtl_dev.recv = rtl8139_recv;

    netdev_register(&rtl_dev);

    put_string("rtl8139: initialised as eth1\n", DEFAULT_ATTR);
}