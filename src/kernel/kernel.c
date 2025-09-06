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

    for (volatile int i = 0; i < 500000000; i++);
    // we just wait a little bit so the user can read the messages
}

int main() {
    bootscreen();

    clearScreen(0x00);
    login();

    for (volatile int i = 0; i < 1000000000; i++);
    //later we just wait until the password was entered


    //PANIC("USB_INIT");
    //kernel_panic_screen("KERNEL PANIC - ERRNO 0X00AAAA", "usb_init (l;devices) at 21",  __FILE__, __LINE__);
    // just a test for panic

    desktop();

    return 0;
}
