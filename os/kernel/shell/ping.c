#include <io/kernel_io.h>
#include <lib/string.h>
#include <net/arp.h>
#include <net/byteorder.h>
#include <net/dns.h>
#include <net/icmp.h>

int cmd_ping(int argc, char **argv) {
    if (argc < 2) {
        put_string("Usage: ping <IP>\n", DEFAULT_ATTR);
        return 1;
    }
    
    uint32_t gateway_ip = htonl(0x0a000202); // QEMU NAT gateway: 10.0.2.2
    char buf2[128];
    snprintf(buf2, sizeof(buf2), "Resolving ARP for %d.%d.%d.%d\n",
        gateway_ip & 0xFF,
        (gateway_ip >> 8) & 0xFF,
        (gateway_ip >> 16) & 0xFF,
        (gateway_ip >> 24) & 0xFF);

    put_string(buf2, DEFAULT_ATTR);

    uint8_t dst_mac[6];
    if (!arp_resolve(gateway_ip, dst_mac)) { // Should be target_ip in real world, gateway_ip in QEMU
        put_string("ARP resolution failed\n", DEFAULT_ATTR);
        return 2;
    }

    uint32_t target_ip = parse_ip(argv[1]);
    if (target_ip == 0) {
        target_ip = dns_resolve(argv[1]);
        if (target_ip == 0) {
            put_string("Invalid IP address\n", DEFAULT_ATTR);
            return 2;
        }
    }

    char buf[128];
    snprintf(buf, sizeof(buf), "Pinging %d.%d.%d.%d\n",
        target_ip & 0xFF,
        (target_ip >> 8) & 0xFF,
        (target_ip >> 16) & 0xFF,
        (target_ip >> 24) & 0xFF);

    put_string(buf, DEFAULT_ATTR);

    for (uint16_t seq = 1; seq <= 4; ++seq) {
        icmp_send_echo(target_ip, seq);
        snprintf(buf, sizeof(buf), "ICMP echo request #%d sent\n", seq);
        put_string(buf, DEFAULT_ATTR);

        for (volatile int i = 0; i < 500000; ++i);
    }

    put_string("Ping test done\n", DEFAULT_ATTR);
    return 0;
}