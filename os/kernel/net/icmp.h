#pragma once
#include <lib/stdint.h>
#include <lib/stddef.h>

#define ICMP_TYPE_ECHO_REQUEST 8
#define ICMP_TYPE_ECHO_REPLY   0

typedef struct {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint16_t identifier;
    uint16_t sequence;
} __attribute__((packed)) icmp_header_t;

void icmp_send_echo(uint32_t dst_ip, uint16_t seq);
void icmp_handle(uint32_t src_ip, uint8_t *packet, size_t len);