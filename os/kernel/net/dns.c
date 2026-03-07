#include <io/kernel_io.h>
#include <lib/string.h>
#include "byteorder.h"
#include "dns.h"
#include "ethernet.h"
#include "ipv4.h"
#include "net.h"
#include "netdev.h"
#include "udp.h"

#define DNS_PORT 53

static uint16_t transaction_id = 0x1234;
static uint32_t resolved_ip = 0;

static void dns_udp_callback(uint16_t src_port, uint16_t dest_port, uint8_t *data, size_t len) {
    if (len < 12) return; // DNS header

    uint16_t resp_id = (data[0] << 8) | data[1];
    if (resp_id != transaction_id) return; // Not ours ;(

    uint16_t ancount = (data[6] << 8) | data[7];
    if (ancount == 0) return;

    size_t pos = 12;
    while (pos < len && data[pos] != 0) {
        pos += data[pos] + 1;
    }
    pos += 5; // null byte + type(2) + class(2)

    // Read first A record
    if (pos + 16 > len) return;

    // Answer type
    uint16_t type = (data[pos+2] << 8) | data[pos+3];
    if (type != 1) return; // type A

    // IP in last 4 bytes of RDATA
    resolved_ip = (data[pos+12] << 24) | (data[pos+13] << 16) | (data[pos+14] << 8) | data[pos+15];
}

void dns_init(void) {
    udp_bind(12345, dns_udp_callback); // Temporary source port bind
}

uint32_t dns_resolve(const char *hostname) {
    resolved_ip = 0;
    transaction_id++;

    uint8_t buf[512];
    memset(buf, 0, sizeof(buf));

    // DNS header
    buf[0] = (transaction_id >> 8) & 0xFF;
    buf[1] = transaction_id & 0xFF;
    buf[2] = 0x01; // Recursion please 🙏
    buf[5] = 0x01; // QDCOUNT = 1

    // Encode le hostname
    const char *label = hostname;
    size_t offset = 12;
    while (*label) {
        const char *next = strchr(label, '.');
        size_t len = next ? (size_t)(next - label) : strlen(label);
        buf[offset++] = len;
        for (size_t i = 0; i < len; i++)
            buf[offset++] = label[i];
        if (!next) break;
        label = next + 1;
    }
    buf[offset++] = 0;
    buf[offset++] = 0; buf[offset++] = 1; // Type A
    buf[offset++] = 0; buf[offset++] = 1; // Class IN

    udp_send(12345, DNS_PORT, g_dns_server, buf, offset);
    
    while (resolved_ip == 0) {
        uint8_t frame[2048];
        int len = netdev_recv(frame, sizeof(frame));
        if (len > 0) {
            put_string("ETH_FRAME RX\n", DEFAULT_ATTR);
            eth_receive(frame, len);
        }
    }

    return resolved_ip;
}