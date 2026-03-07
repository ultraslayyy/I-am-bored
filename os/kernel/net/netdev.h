#pragma once
#include <lib/stddef.h>
#include <lib/stdint.h>

typedef struct netdev {
    char name[8];
    uint8_t mac[6];

    int (*send)(struct netdev *dev, const void *data, size_t len);
    int (*recv)(struct netdev *dev, void *buf, size_t max_len);

    void *priv;
} netdev_t;

void netdev_init(void);
void netdev_register(netdev_t *dev);
int netdev_send(const void *data, size_t len);
int netdev_recv(void *buf, size_t max_len);
uint8_t *netdev_get_mac(void);
netdev_t *netdev_get(void);