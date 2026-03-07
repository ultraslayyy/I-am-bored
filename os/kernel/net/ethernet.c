#include <lib/stddef.h>
#include <lib/stdint.h>
#include <lib/string.h>
#include <io/kernel_io.h>
#include "arp.h"
#include "byteorder.h"
#include "ethernet.h"
#include "ipv4.h"
#include "netdev.h"

void eth_receive(uint8_t *frame, size_t len) {
    if (len < sizeof(eth_header_t)) return;

    eth_header_t *eth = (eth_header_t *)frame;
    uint16_t type = bswap16(eth->ethertype);

    uint8_t *payload = frame + sizeof(eth_header_t);
    size_t payload_len = len - sizeof(eth_header_t);

    switch (type) {
        case ETH_TYPE_ARP:
            arp_handle(payload, payload_len);
            break;
        case ETH_TYPE_IPV4:
            ipv4_receive(payload, payload_len);
            break;
    }
}

void eth_send(uint8_t *payload, size_t len, uint16_t ethertype, uint8_t *dst_mac) {
    uint8_t frame[1518]; // Can't remember where 1518 came from, but works ig

    eth_header_t *eth = (eth_header_t *)frame;
    memcpy(eth->dst, dst_mac, 6);
    memcpy(eth->src, netdev_get_mac(), 6);
    eth->ethertype = bswap16(ethertype);

    memcpy(frame + sizeof(eth_header_t), payload, len);

    put_string("NETDEV_SEND\n", DEFAULT_ATTR);
    netdev_send(frame, len + sizeof(eth_header_t));
}