#include <lib/string.h>
#include <lib/stdint.h>
#include <lib/stddef.h>
#include <io/kernel_io.h>
#include "arp.h"
#include "byteorder.h"
#include "ethernet.h"
#include "netdev.h"

#define ARP_HW_ETHERNET 1
#define ARP_PROTO_IPV4  0x0800
#define ARP_OP_REQUEST  1
#define ARP_OP_REPLY    2

typedef struct {
    uint16_t hw_type;
    uint16_t proto_type;
    uint8_t  hw_len;
    uint8_t  proto_len;
    uint16_t opcode;
    uint8_t  sender_mac[6];
    uint32_t sender_ip;
    uint8_t  target_mac[6];
    uint32_t target_ip;
} __attribute__((packed)) arp_packet_t;

static arp_entry_t arp_cache[ARP_CACHE_SIZE];
static uint32_t local_ip;
static uint8_t local_mac[6];

static void arp_cache_insert(uint32_t ip, uint8_t *mac) {
    for (size_t i = 0; i < ARP_CACHE_SIZE; ++i) {
        if (arp_cache[i].valid && arp_cache[i].ip == ip) {
            memcpy(arp_cache[i].mac, mac, 6);
            return;
        }
    }

    for (size_t i = 0; i < ARP_CACHE_SIZE; ++i) {
        if (!arp_cache[i].valid) {
            arp_cache[i].ip = ip;
            memcpy(arp_cache[i].mac, mac, 6);
            arp_cache[i].valid = 1;
            return;
        }
    }

    static int next_victim = 0;
    arp_cache[next_victim].ip = ip;
    memcpy(arp_cache[next_victim].mac, mac, 6);
    arp_cache[next_victim].valid = 1;
    next_victim = (next_victim + 1) % ARP_CACHE_SIZE;
}


void arp_init(uint32_t ip, uint8_t *mac) {
    local_ip = ip;
    memcpy(local_mac, mac, 6);

    for (size_t i = 0; i < ARP_CACHE_SIZE; ++i) {
        arp_cache[i].valid = 0;
    }
}

void arp_handle(uint8_t *frame, size_t len) {
    if (len < sizeof(arp_packet_t)) return;

    arp_packet_t *arp = (arp_packet_t *)frame;

    if (bswap16(arp->hw_type) != ARP_HW_ETHERNET) return;
    if (bswap16(arp->proto_type) != ARP_PROTO_IPV4) return;
    if (arp->hw_len != 6 || arp->proto_len != 4) return;

    uint16_t opcode = bswap16(arp->opcode);
    uint32_t target_ip = arp->target_ip;

    arp_cache_insert(arp->sender_ip, arp->sender_mac);

    if (opcode != ARP_OP_REQUEST) return;
    if (target_ip != local_ip) return;

    put_string("ARP request for us\n", DEFAULT_ATTR);

    arp_packet_t reply;
    reply.hw_type    = bswap16(ARP_HW_ETHERNET);
    reply.proto_type = bswap16(ARP_PROTO_IPV4);
    reply.hw_len     = 6;
    reply.proto_len  = 4;
    reply.opcode     = bswap16(ARP_OP_REPLY);

    memcpy(reply.sender_mac, local_mac, 6);
    reply.sender_ip = local_ip;

    memcpy(reply.target_mac, arp->sender_mac, 6);
    reply.target_ip = arp->sender_ip;

    eth_send((uint8_t *)&reply, sizeof(reply), ETH_TYPE_ARP, arp->sender_mac);
}

static int arp_cache_lookup(uint32_t ip, uint8_t *mac_out) {
    for (size_t i = 0; i < ARP_CACHE_SIZE; ++i) {
        if (arp_cache[i].valid && arp_cache[i].ip == ip) {
            memcpy(mac_out, arp_cache[i].mac, 6);
            return 1;
        }
    }
    return 0;
}

static void arp_send_request(uint32_t target_ip) {
    arp_packet_t pkt;

    pkt.hw_type    = bswap16(ARP_HW_ETHERNET);
    pkt.proto_type = bswap16(ARP_PROTO_IPV4);
    pkt.hw_len     = 6;
    pkt.proto_len  = 4;
    pkt.opcode     = bswap16(ARP_OP_REQUEST);

    memcpy(pkt.sender_mac, local_mac, 6);
    pkt.sender_ip = local_ip;

    memset(pkt.target_mac, 0, 6);
    pkt.target_ip = target_ip;

    uint8_t broadcast[6] = {0xff,0xff,0xff,0xff,0xff,0xff};

    put_string("ETH_SEND\n", DEFAULT_ATTR);
    eth_send((uint8_t *)&pkt, sizeof(pkt), ETH_TYPE_ARP, broadcast);
}

int arp_resolve(uint32_t ip, uint8_t *mac_out) {
    if (ip == 0xFFFFFFFF) {
        memset(mac_out, 0xFF, 6);
        return 1;
    }

    put_string("Resolving ARP\n", DEFAULT_ATTR);
    if (arp_cache_lookup(ip, mac_out)) {
        return 1;
    }

    put_string("Sending ARP request\n", DEFAULT_ATTR);
    arp_send_request(ip);

    put_string("Waiting for ARP reply\n", DEFAULT_ATTR);
    for (volatile int i = 0; i < 10000; ++i) {
        if (arp_cache_lookup(ip, mac_out)) {
            return 1;
        }
        
        uint8_t frame[2048];
        int len = netdev_recv(frame, sizeof(frame));
        if (len > 0) {
            eth_header_t *eth = (eth_header_t *)frame;
            if (bswap16(eth->ethertype) == ETH_TYPE_ARP) {
                arp_handle(frame + sizeof(eth_header_t), len - sizeof(eth_header_t));
            }
        }
    }

    return 0;
}
