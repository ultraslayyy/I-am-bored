#include <io/kernel_io.h>
#include <lib/string.h>
#include "byteorder.h"
#include "dhcp.h"
#include "ipv4.h"
#include "net.h"
#include "udp.h"

static uint8_t  client_mac[6];
static uint32_t transaction_id = 0x12345678; // Fixed for now, randomise later
static uint32_t offered_ip = 0;
static uint32_t server_ip = 0;
static uint32_t subnet_mask = 0;
static uint32_t router_ip = 0;
static uint32_t dns_ip = 0;

void dhcp_init(uint8_t mac[6]) {
    memcpy(client_mac, mac, 6);
    transaction_id = 0x12345678; // Fixed for now
}

static size_t dhcp_add_option(uint8_t *options, size_t idx, uint8_t type, uint8_t len, uint8_t *data) {
    options[idx++] = type;
    options[idx++] = len;
    memcpy(options + idx, data, len);
    return idx + len;
}

void dhcp_start(void) {
    dhcp_packet_t pkt;
    memset(&pkt, 0, sizeof(pkt));

    pkt.op = 1;
    pkt.htype = 1;
    pkt.hlen = 6;
    pkt.xid = htonl(transaction_id);
    pkt.flags = htons(0x8000);
    memcpy(pkt.chaddr, client_mac, 6);
    pkt.magic = htonl(DHCP_MAGIC);

    size_t idx = 0;
    uint8_t msg_type = DHCP_DISCOVER;
    idx = dhcp_add_option(pkt.options, idx, 53, 1, &msg_type); // Message type
    uint8_t end = 255;
    pkt.options[idx++] = end;

    uint8_t broadcast_mac[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
    (void)broadcast_mac;
    udp_send(DHCP_CLIENT_PORT, DHCP_SERVER_PORT, 0xFFFFFFFF, (uint8_t*)&pkt, sizeof(pkt));

    put_string("DHCPDISCOVER sent\n", DEFAULT_ATTR);
}

void dhcp_receive(uint16_t src_port, uint16_t dst_port, uint8_t *data, size_t len) {
    (void)src_port;
    (void)dst_port;

    // Where in the hell did I get 'options' from it doesn't exist
    // What drugs was I on coding this, but idk code works so I can't be bothered to remove
    if (len < offsetof(dhcp_packet_t, options)) return;
    dhcp_packet_t *pkt = (dhcp_packet_t *)data;

    if (ntohl(pkt->xid) != transaction_id) return;
    if (ntohl(pkt->magic) != DHCP_MAGIC) return;

    uint8_t *opt = pkt->options;
    size_t i = 0;
    uint8_t msg_type = 0;
    uint32_t server_ip_opt = 0;
    size_t options_len = len - offsetof(dhcp_packet_t, options);

    while (i < options_len && opt[i] != 255) {
        uint8_t type = opt[i++];
        if (i >= options_len) break;
        uint8_t length = opt[i++];
        if (i + length > options_len) break;
        
        if (type == 53 && length == 1) {
            msg_type = opt[i];
        } else if (type == 54 && length == 4) {
            memcpy(&server_ip_opt, &opt[i], 4);
        } else if (type == 1 && length == 4) {
            memcpy(&subnet_mask, &opt[i], 4);
        } else if (type == 3 && length >= 4) {
            memcpy(&router_ip, &opt[i], 4);
        } else if (type == 6 && length >= 4) {
            memcpy(&dns_ip, &opt[i], 4);
        }

        i += length;
    }

    if (msg_type == DHCP_OFFER) {
        offered_ip = pkt->yiaddr;
        server_ip  = server_ip_opt ? server_ip_opt : pkt->siaddr;

        char buf[128];
        snprintf(buf, sizeof(buf), "DHCPOFFER received, offered IP: %d.%d.%d.%d\n",
            offered_ip & 0xFF,
            (offered_ip >> 8) & 0xFF,
            (offered_ip >> 16) & 0xFF,
            (offered_ip >> 24) & 0xFF);
        put_string(buf, DEFAULT_ATTR);

        dhcp_packet_t req;
        memset(&req, 0, sizeof(req));
        req.op = 1;
        req.htype = 1;
        req.hlen = 6;
        req.xid = htonl(transaction_id);
        req.flags = htons(0x8000);
        memcpy(req.chaddr, client_mac, 6);
        req.magic = htonl(DHCP_MAGIC);

        size_t idx2 = 0;
        uint8_t req_type = DHCP_REQUEST;
        idx2 = dhcp_add_option(req.options, idx2, 53, 1, &req_type);
        uint8_t ip_bytes[4] = {
            (offered_ip >> 0) & 0xFF,
            (offered_ip >> 8) & 0xFF,
            (offered_ip >> 16) & 0xFF,
            (offered_ip >> 24) & 0xFF
        };
        idx2 = dhcp_add_option(req.options, idx2, 50, 4, ip_bytes); // Requested IP

        uint8_t server_bytes[4] = {
            (server_ip >> 0) & 0xFF,
            (server_ip >> 8) & 0xFF,
            (server_ip >> 16) & 0xFF,
            (server_ip >> 24) & 0xFF
        };
        idx2 = dhcp_add_option(req.options, idx2, 54, 4, server_bytes); // Server Identifier

        uint8_t end = 255;
        req.options[idx2++] = end;

        udp_send(DHCP_CLIENT_PORT, DHCP_SERVER_PORT, 0xFFFFFFFF, (uint8_t *)&req, sizeof(req));
        put_string("DHCPREQUEST sent\n", DEFAULT_ATTR);
    } else if (msg_type == DHCP_ACK) {
        char buf[128];
        snprintf(buf, sizeof(buf), "DHCPACK received, IP assigned: %d.%d.%d.%d\n",
            offered_ip & 0xFF,
            (offered_ip >> 8) & 0xFF,
            (offered_ip >> 16) & 0xFF,
            (offered_ip >> 24) & 0xFF);
        put_string(buf, DEFAULT_ATTR);

        ipv4_set_addr(offered_ip);
        g_subnet_mask = subnet_mask;
        g_gateway_ip = router_ip;
        g_dns_server = dns_ip;
    }
}