#include "../include/io.h"
#include "../include/fb.h"
#include "../include/desktop.h"
#include "../include/login_window.h"
#include "panic.h"

void bootscreen() {
    uart_init();

    fb_init();

    clearScreen(0x00);

    drawStringSized(10, 10, "uart initialized", 0x0F, 12);
    drawStringSized(10, 25, "fb initialized", 0x0F, 12);
    drawStringSized(10, 40, "starting kernel", 0x0F, 12);
}

int main() {
    bootscreen();

    clearScreen(0x00);

    login();

    for (volatile int i = 0; i < 1000000000; i++);

    //PANIC("Login failed");
    kernel_panic_screen("KERNEL PANIC - ERRNO 0X00AAAA", "Segment fault", "doch", 5);

    for (volatile int i = 0; i < 1000000000; i++);

    clearScreen(0x00);
    desktop();

    return 0;
}
