#include <io/kernel_io.h>
#include <lib/string.h>
#include "byteorder.h"
#include "ipv4.h"
#include "udp.h"

#define MAX_UDP_PORTS 16

typedef struct {
    uint16_t port;
    udp_handler_t handler;
} udp_entry_t;

static udp_entry_t udp_ports[MAX_UDP_PORTS];

void udp_bind(uint16_t port, udp_handler_t handler) {
    for (size_t i = 0; i < MAX_UDP_PORTS; ++i) {
        if (udp_ports[i].handler == 0) {
            udp_ports[i].port = port;
            udp_ports[i].handler = handler;
            return;
        }
    }
}

void udp_handle_packet(uint16_t src_port, uint16_t dst_port, uint8_t *data, size_t len) {
    for (size_t i = 0; i < MAX_UDP_PORTS; ++i) {
        if (udp_ports[i].handler && udp_ports[i].port == dst_port) {
            udp_ports[i].handler(src_port, dst_port, data, len);
            return;
        }
    }

    char buf[64];
    snprintf(buf, sizeof(buf), "UDP packet to port %d dropped\n", dst_port);
    put_string(buf, DEFAULT_ATTR);
}

void udp_send(uint16_t src_port, uint16_t dst_port, uint32_t dst_ip, uint8_t *payload, size_t len) {
    if (sizeof(udp_header_t) + len > 1500) {
        put_string("UDP payload too large\n", DEFAULT_ATTR);
        return;
    }

    uint8_t buffer[1500];
    udp_header_t *udp = (udp_header_t *)buffer;

    udp->src_port = htons(src_port);
    udp->dst_port = htons(dst_port);
    udp->length   = htons(sizeof(udp_header_t) + len);
    udp->checksum = 0; // Skip for now

    memcpy(buffer + sizeof(udp_header_t), payload, len);

    ipv4_send(dst_ip, IPV4_PROTO_UDP, buffer, sizeof(udp_header_t) + len);
    put_string("UDP SENT\n", DEFAULT_ATTR);
}