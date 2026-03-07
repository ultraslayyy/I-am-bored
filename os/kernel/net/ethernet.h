#pragma once
#include <lib/stddef.h>
#include <lib/stdint.h>

#define ETH_TYPE_IPV4 0x0800
#define ETH_TYPE_ARP  0x0806
#define ETH_ADDR_LEN  6

typedef struct {
    uint8_t dst[ETH_ADDR_LEN];
    uint8_t src[ETH_ADDR_LEN];
    uint16_t ethertype;
} __attribute__((packed)) eth_header_t;

void eth_send(uint8_t *payload, size_t len, uint16_t ethertype, uint8_t *dst_mac);
void eth_receive(uint8_t *frame, size_t len);