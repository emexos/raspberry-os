// In src/kernel/kernel.c - USB Integration hinzufügen

#include "../include/io.h"
#include "../include/fb.h"
#include "../include/desktop.h"
#include "../include/login_window.h"
#include "../include/usb.h"  // Neuer USB Header
#include "panic.h"

void bootscreen() {
    uart_init();
    fb_init();
    clearScreen(0x00);

    drawStringSized(10, 10, "uart initialized", 0x0F, 12);
    drawStringSized(10, 25, "fb initialized", 0x0F, 12);
    drawStringSized(10, 40, "starting kernel", 0x0F, 12);

    // USB Subsystem initialisieren
    drawStringSized(10, 55, "initializing usb...", 0x0E, 12);
    if (usb_init()) {
        drawStringSized(10, 70, "usb initialized", 0x0A, 12);

        // USB Status anzeigen
        usb_status_info_t status;
        usb_get_status(&status);

        if (status.xhci_initialized) {
            drawStringSized(10, 85, "xhci controller found", 0x0A, 12);
        } else {
            drawStringSized(10, 85, "no xhci controller", 0x0C, 12);
        }

        // PCIe Status anzeigen
        if (status.pcie_device_count > 0) {
            drawStringSized(10, 100, "pcie devices detected", 0x0A, 12);
        } else {
            drawStringSized(10, 100, "no pcie devices", 0x0C, 12);
        }

    } else {
        drawStringSized(10, 70, "usb init failed", 0x0C, 12);
    }

    // Warten damit man die Meldungen lesen kann
    for (volatile int i = 0; i < 500000000; i++);
}

void print_to_uart(const char* msg) {
    uart_writeText((char*)msg);
}

int main() {
    bootscreen();

    // USB Informationen über UART ausgeben
    uart_writeText("\n=== emexOS USB Subsystem ===\n");
    usb_print_info(print_to_uart);

    clearScreen(0x00);
    login();

    for (volatile int i = 0; i < 1000000000; i++);

    //PANIC("Login failed");
    kernel_panic_screen("KERNEL PANIC - ERRNO 0X00AAAA", "Segment fault", "doch", 5);

    for (volatile int i = 0; i < 1000000000; i++);

    clearScreen(0x00);

    // USB System vor Desktop-Start aktualisieren
    usb_update();
    usb_enumerate_devices();

    desktop();

    return 0;
}
