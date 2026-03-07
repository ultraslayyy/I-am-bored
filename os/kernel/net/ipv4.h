#pragma once
#include <lib/stdint.h>
#include <lib/stddef.h>

#define IPV4_PROTO_ICMP 1

typedef struct {
    uint8_t version_ihl;
    uint8_t tos;
    uint16_t total_length;
    uint16_t id;
    uint16_t flags_fragment;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t checksum;
    uint32_t src_ip;
    uint32_t dst_ip;
} __attribute__((packed)) ipv4_header_t;

void ipv4_send(uint32_t dst_ip, uint8_t protocol, uint8_t *payload, size_t len);
void ipv4_receive(uint8_t *packet, size_t len);
void ipv4_set_addr(uint32_t ip);