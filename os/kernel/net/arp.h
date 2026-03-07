#pragma once
#include <lib/stddef.h>
#include <lib/stdint.h>

#define ARP_CACHE_SIZE 4
typedef struct {
    uint32_t ip;
    uint8_t mac[6];
    int valid;
} arp_entry_t;

int arp_resolve(uint32_t ip, uint8_t *mac_out);
void arp_handle(uint8_t *frame, size_t len);
void arp_init(uint32_t ip, uint8_t *mac);