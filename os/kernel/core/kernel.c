#include <block/block.h>
#include <drivers/ata/ata.h>
#include <drivers/memory/memory.h>
#include <fs/fd.h>
#include <fs/ramfs.h>
#include <fs/vfs.h>
#include <fs/fat16/fat16.h>
#include <io/kernel_io.h>
#include <lib/string.h>
#include <shell/shell.h>
#include <drivers/video/vesa.h>
#include "boot_info.h"

extern uint32_t kernel_stack_end;

/* static void print_hex_byte(uint8_t value) {
    const char *hex = "0123456789ABCDEF";

    char out[3];
    out[0] = hex[(value >> 4) & 0xF];
    out[1] = hex[value & 0xF];
    out[2] = '\0';

    put_string(out, DEFAULT_ATTR);
} */

#include <drivers/net/e1000.h>
#include <drivers/net/rtl8139.h>
#include <net/arp.h>
#include <net/byteorder.h>
#include <net/dhcp.h>
#include <net/dns.h>
#include <net/ethernet.h>
#include <net/ipv4.h>
#include <net/net.h>
#include <net/netdev.h>
#include <net/udp.h>
void kernel_main(boot_info_t *mbi) {
    gdt_init();
    tss_init((uint32_t)&kernel_stack_end);
    
    memory_init(mbi);
    paging_init();

    vesa_init(mbi);
    clear_screen();
    
    // vesa_draw_rect(10, 10, 100, 100, 0xFF0000); // Red square
    // vesa_draw_rect(120, 10, 100, 100, 0x00FF00); // Green square
    // vesa_draw_rect(230, 10, 100, 100, 0x0000FF); // Blue square

    put_string("Memory initialised\n", DEFAULT_ATTR);

    e1000_init();
    rtl8139_init();
    uint8_t mac[6];
    memcpy(mac, netdev_get_mac(), 6);
    arp_init(0, mac);
    ipv4_set_addr(0);

    udp_bind(68, dhcp_receive);

    dhcp_init(mac);
    dhcp_start();

    while (g_ip_addr == 0) {
        uint8_t frame[2048];
        int len = netdev_recv(frame, sizeof(frame));
        if (len > 0) {
            put_string("ETH FRAME RX\n", DEFAULT_ATTR);
            eth_receive(frame, len);
        }
    }

    char buf[128];
    snprintf(buf, sizeof(buf), "IP %d.%d.%d.%d\n", 
        g_ip_addr & 0xFF,
        (g_ip_addr >> 8) & 0xFF,
        (g_ip_addr >> 16) & 0xFF,
        (g_ip_addr >> 24) & 0xFF
    );
    put_string(buf, DEFAULT_ATTR);

    dns_init();

    ata_init();
    vfs_init();
    // ramfs_init();
    block_device_t *dev = block_get_device();
    fat16_init(dev);
    fd_init();

    char path[MAX_PATH_LEN];
    vfs_get_path(kernel_cwd, path, sizeof(path));

    put_string(path, DEFAULT_ATTR);
    put_char(' ', DEFAULT_ATTR);
    const char *prompt = "$ ";
    put_string(prompt, DEFAULT_ATTR);

    idt_init();
    syscall_init();
    scheduler_init();

    pit_init(100);

    while (1) {
        __asm__ volatile("hlt");
        uint8_t frame[2048];
        int len = netdev_recv(frame, sizeof(frame));
        if (len > 0) {
            eth_receive(frame, len);
        }
    }
}