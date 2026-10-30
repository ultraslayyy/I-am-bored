#pragma once
#include <lib/stdint.h>

#define UHCI_PCI_CLASS    0x0C
#define UHCI_PCI_SUBCLASS 0x03
#define UHCI_PCI_PROGIF    0x00

#define UHCI_REG_USBCMD    0x00
#define UHCI_REG_USBSTS    0x02
#define UHCI_REG_USBINTR   0x04
#define UHCI_REG_FRNUM     0x06
#define UHCI_REG_FRBASEADD 0x08
#define UHCI_REG_SOFMOD    0x0C
#define UHCI_REG_PORTSC1   0x10
#define UHCI_REG_PORTSC2   0x12

#define UHCI_CMD_RS      (1 << 0)
#define UHCI_CMD_HCRESET (1 << 1)
#define UHCI_CMD_GRESET  (1 << 2)
#define UHCI_CMD_GSUSP   (1 << 3)
#define UHCI_CMD_EGSM    (1 << 4)
#define UHCI_CMD_CONFIG  (1 << 6)
#define UHCI_CMD_MAXP    (1 << 7)

#define UHCI_STS_USBINT (1 << 0)
#define UHCI_STS_ERROR  (1 << 1)
#define UHCI_STS_RD     (1 << 2)
#define UHCI_STS_HSE    (1 << 3)
#define UHCI_STS_HCPE   (1 << 4)
#define UHCI_STS_HCH    (1 << 5)

#define UHCI_INTR_TIMEOUT (1 << 0)
#define UHCI_INTR_RESUME  (1 << 1)
#define UHCI_INTR_IOC     (1 << 2)
#define UHCI_INTR_SP      (1 << 3)

#define UHCI_PORT_CCS     (1 << 0)
#define UHCI_PORT_CSC     (1 << 1)
#define UHCI_PORT_PE      (1 << 2)
#define UHCI_PORT_PEC     (1 << 3)
#define UHCI_PORT_LS_MASK (1 << 4)
#define UHCI_PORT_RD      (1 << 6)
#define UHCI_PORT_LSDA    (1 << 8)
#define UHCI_PORT_RESET   (1 << 9)
#define UHCI_PORT_SUSPEND (1 << 12)

#define UHCI_FRAME_COUNT 1024

#define UHCI_LINK_TERMINATE 0x00000001
#define UHCI_LINK_QH        0x00000002
#define UHCI_LINK_DEPTH     0x00000004

#define UHCI_TD_ACTIVE     (1u << 23)
#define UHCI_TD_STALLED    (1u << 22)
#define UHCI_TD_DBUFFER    (1u << 21)
#define UHCI_TD_BABBLE     (1u << 20)
#define UHCI_TD_NAK        (1u << 19)
#define UHCI_TD_CRCTIMEO   (1u << 18)
#define UHCI_TD_BITSTUFF   (1u << 17)
#define UHCI_TD_C_ERR_MASK (3u << 27)
#define UHCI_TD_IOC        (1u << 24)
#define UHCI_TD_LS         (1u << 26)
#define UHCI_TD_SPD        (1u << 29)

#define UHCI_PID_SETUP 0x2D
#define UHCI_PID_IN    0x69
#define UHCI_PID_OUT   0xE1

#define UHCI_MAX_TDS 34

typedef struct {
    uint16_t io_base;
    uint8_t  initialised;

    uint32_t *frame_list;
} uhci_controller_t;

typedef struct {
    uint32_t link;
    volatile uint32_t status;
    uint32_t token;
    uint32_t buffer;
} __attribute__((packed, aligned(16))) uhci_td_t;

typedef struct {
    uint32_t head;
    uint32_t element;
    uint32_t reserved[2];
} __attribute__((packed, aligned(16))) uhci_qh_t;

int uhci_init(void);
const uhci_controller_t *uhci_get_controller(void);

uint16_t uhci_read16(uint16_t reg);
uint32_t uhci_read32(uint16_t reg);

void uhci_write16(uint16_t reg, uint16_t value);
void uhci_write32(uint16_t reg, uint32_t value);

int uhci_reset(void);
void uhci_dump_ports(void);
int uhci_reset_port(unsigned int port);