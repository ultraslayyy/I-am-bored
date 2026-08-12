#pragma once

int acpi_init(void *rsdp_ptr);
void acpi_shutdown(void);
void acpi_reboot(void);