#pragma once
#include <lib/stdint.h>

void dns_init(void);
uint32_t dns_resolve(const char *hostname);