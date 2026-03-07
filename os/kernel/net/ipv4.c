#include <io/kernel_io.h>
#include <lib/stddef.h>
#include <lib/stdint.h>
#include <lib/string.h>
#include "arp.h"
#include "byteorder.h"
#include "ethernet.h"
#include "icmp.h"
#include "ipv4.h"
#include "net.h"
#include "udp.h"

static uint16_t checksum16(void *data, size_t len) {
    uint32_t sum = 0;
    uint16_t *ptr = data;

    while (len > 1) {
        sum += *ptr++;
        len -= 2;
    }

    if (len) {
        sum += *(uint8_t *)ptr;
    }

    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return ~sum;
}

void ipv4_set_addr(uint32_t ip) {
    g_ip_addr = ip;
}

void ipv4_receive(uint8_t *packet, size_t len) {
    if (len < sizeof(ipv4_header_t)) return;

    ipv4_header_t *ip = (ipv4_header_t *)packet;
    uint8_t ihl = (ip->version_ihl & 0x0F) * 4;

    if (ihl < sizeof(ipv4_header_t)) return;
    if (len < ihl) return;

    if (ip->dst_ip != g_ip_addr && ip->dst_ip != 0xFFFFFFFF) return;

    uint16_t old = ip->checksum;
    ip->checksum = 0;
    uint16_t calc = checksum16(ip, ihl);
    ip->checksum = old;
    if (calc != old) return;

    uint8_t *payload = packet + ihl;
    size_t payload_len = ntohs(ip->total_length) - ihl;

    switch (ip->protocol) {
        case IPV4_PROTO_ICMP:
            icmp_handle(ip->src_ip, payload, payload_len);
            break;
        case IPV4_PROTO_UDP:
            if (payload_len < sizeof(udp_header_t)) break;
            udp_header_t *udp = (udp_header_t *)payload;
            uint16_t src_port = ntohs(udp->src_port);
            uint16_t dst_port = ntohs(udp->dst_port);
            uint8_t *udp_payload = payload + sizeof(udp_header_t);
            size_t udp_len = ntohs(udp->length) - sizeof(udp_header_t);

            udp_handle_packet(src_port, dst_port, udp_payload, udp_len);
            break;
    }
}

void ipv4_send(uint32_t dst_ip, uint8_t protocol, uint8_t *payload, size_t len) {
    if (sizeof(ipv4_header_t) + len > 1500) return;

    uint8_t buffer[1500];
    ipv4_header_t *ip = (ipv4_header_t *)buffer;

    ip->version_ihl = 0x45;
    ip->tos = 0;
    ip->total_length = htons(sizeof(ipv4_header_t) + len);
    ip->id = 0;
    ip->flags_fragment = htons(0x4000);
    ip->ttl = 64;
    ip->protocol = protocol;
    ip->checksum = 0;
    ip->src_ip = g_ip_addr;
    ip->dst_ip = dst_ip;

    ip->checksum = checksum16(ip, sizeof(ipv4_header_t));

    memcpy(buffer + sizeof(ipv4_header_t), payload, len);

    uint32_t next_hop;
    if (g_gateway_ip == 0) {
        next_hop = dst_ip;
    } else if ((dst_ip & g_subnet_mask) == (g_ip_addr & g_subnet_mask)) {
        next_hop = dst_ip;
    } else {
        next_hop = g_gateway_ip;
    }

    uint8_t dst_mac[6];
    put_string("Resolving ARP\n", DEFAULT_ATTR);
    if (!arp_resolve(next_hop, dst_mac)) {
        return;
    }

    put_string("ETH_SEND\n", DEFAULT_ATTR);
    eth_send(buffer, sizeof(ipv4_header_t) + len, ETH_TYPE_IPV4, dst_mac);
}
