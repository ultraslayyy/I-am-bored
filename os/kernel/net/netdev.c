#include "netdev.h"

static netdev_t *root_netdev = 0;

void netdev_init(void) {
    root_netdev = 0;
}

netdev_t *netdev_get(void) {
    return root_netdev;
}

void netdev_register(netdev_t *dev) {
    root_netdev = dev;
}

int netdev_send(const void *data, size_t len) {
    if (!root_netdev || !root_netdev->send) {
        return -1;
    }

    return root_netdev->send(root_netdev, data, len);
}

int netdev_recv(void *buf, size_t max_len) {
    if (!root_netdev || !root_netdev->recv) {
        return -1;
    }

    return root_netdev->recv(root_netdev, buf, max_len);
}

uint8_t *netdev_get_mac(void) {
    if (!root_netdev) {
        return 0;
    }

    return root_netdev->mac;
}