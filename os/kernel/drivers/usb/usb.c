#include <io/kernel_io.h>
#include "ehci.h"
#include "uhci.h"
#include "usb.h"

void usb_init(void) {
    /*
    if (!ehci_init()) {
        put_string("EHCI not found\n", DEFAULT_ATTR);
        return;
    }

    put_string("EHCI initialised\n", DEFAULT_ATTR);

    ehci_probe_ports();
    */

    if (!uhci_init()) {
        put_string("UHCI not found\n", DEFAULT_ATTR);
    }

    put_string("UHCI initialised\n", DEFAULT_ATTR);
}