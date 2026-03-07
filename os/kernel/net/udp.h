#pragma once
#include <lib/stddef.h>
#include <lib/stdint.h>

#define IPV4_PROTO_UDP 17

typedef struct {
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t length;
    uint16_t checksum;
} __attribute__((packed)) udp_header_t;

typedef void (*udp_handler_t)(uint16_t src_port, uint16_t dst_port, uint8_t *data, size_t len);

void udp_send(uint16_t src_port, uint16_t dst_port, uint32_t dst_ip, uint8_t *payload, size_t len);
void udp_bind(uint16_t port, udp_handler_t handler);
void udp_handle_packet(uint16_t src_port, uint16_t dst_port, uint8_t *data, size_t len);