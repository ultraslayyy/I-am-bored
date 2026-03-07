#include <lib/string.h>
#include "byteorder.h"
#include "icmp.h"
#include "ipv4.h"
#include <io/kernel_io.h>

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

void icmp_handle(uint32_t src_ip, uint8_t *packet, size_t len) {
    if (len < sizeof(icmp_header_t)) return;

    icmp_header_t *icmp = (icmp_header_t *)packet;

    if (icmp->type == ICMP_TYPE_ECHO_REPLY) {
        char buf[64];
        snprintf(buf, sizeof(buf),
            "Reply from %d.%d.%d.%d seq=%d\n",
            (src_ip >> 24) & 0xFF, (src_ip >> 16) & 0xFF,
            (src_ip >> 8) & 0xFF, src_ip & 0xFF,
            ntohs(icmp->sequence));
        put_string(buf, DEFAULT_ATTR);
        return;
    }

    if (icmp->type != ICMP_TYPE_ECHO_REQUEST) {
        return;
    }

    icmp->type = ICMP_TYPE_ECHO_REPLY;
    icmp->checksum = 0;
    icmp->checksum = checksum16(icmp, len);

    ipv4_send(src_ip, IPV4_PROTO_ICMP, packet, len);
}

void icmp_send_echo(uint32_t dst_ip, uint16_t seq) {
    uint8_t buffer[64];
    icmp_header_t *icmp = (icmp_header_t *)buffer;

    icmp->type = ICMP_TYPE_ECHO_REQUEST;
    icmp->code = 0;
    icmp->identifier = htons(1);
    icmp->sequence = htons(seq);
    icmp->checksum = 0;

    memset(buffer + sizeof(*icmp), 0xAB, 32);

    size_t len = sizeof(*icmp) + 32;
    icmp->checksum = checksum16(icmp, len);

    put_string("ICMP IPV4_SEND\n", DEFAULT_ATTR);
    ipv4_send(dst_ip, IPV4_PROTO_ICMP, buffer, len);
}
