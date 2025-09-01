#include "../include/fb.h"
#include "panic.h"

static inline int clamp(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

void drawLogoE(int x1, int y1, int x2, int y2) {
    int w = x2 - x1;
    int h = y2 - y1;
    int thickness = h / 6;

    drawRoundedRect(x1, y1, x2, y2, h/6, 0xff, 1, 0xff, 0);

    // Linker Balken
    drawRect(x1 + w/6, y1 + h/6, x1 + w/3, y2 - h/6, 0x00, 1);

    // Obere Linie
    drawRect(x1 + w/6, y1 + h/6, x2 - w/6, y1 + h/6 + thickness, 0x00, 1);

    // Mittlere Linie
    drawRect(x1 + w/6, y1 + h/2 - thickness/2, x2 - w/3, y1 + h/2 + thickness/2, 0x00, 1);

    // Untere Linie
    drawRect(x1 + w/6, y2 - h/6 - thickness, x2 - w/6, y2 - h/6, 0x00, 1);
}

void drawEmexLogo() {
    int logo_w = 250;
    int logo_h = 250;

    int x2 = SCREEN_WIDTH - 100;
    int x1 = x2 - logo_w;
    int y1 = (SCREEN_HEIGHT / 2) - (logo_h / 2);
    int y2 = y1 + logo_h;

    drawLogoE(x1, y1, x2, y2);
}

void kernel_panic_screen(const char* title, const char* reason, const char* file, int line) {
    const unsigned char bg  = 6;   // (6=light_blue,1=red,4=darkblue,9=light_red)
    const unsigned char card_fill = 0;    // black
    const unsigned char text  = 15;       // white
    const unsigned char subtext = 14;     // yellow

    clearScreen(bg);

    for (volatile int i = 0; i < 111500000; i++);

    int card_margin = 80;
    int card_radius = 20;
    int card_border_thick = 3;
    int x1 = card_margin;
    int y1 = card_margin;
    int x2 = SCREEN_WIDTH - card_margin;
    int y2 = SCREEN_HEIGHT - card_margin;

    drawRoundedRect(x1, y1, x2, y2, card_radius, card_fill, 1, 00, 0);

    drawStringSized(100, 100, (char*)title, 15, 35);
    drawStringSized(100, 150, (char*)reason, 0x0e, 24);
    drawStringSized(100, 200, (char*)file, 0x0e, 24);

    drawEmexLogo();

    drawStringSized(100, 960, (char*)"emexOS rpi4 - system halted.", 15, 16);

    for (;;) {
        asm volatile("wfi");
    }
}
