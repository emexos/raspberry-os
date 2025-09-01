#include "../include/fb.h"
#include "panic.h"

static inline int clamp(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

void kernel_panic_screen(const char* title, const char* reason, const char* file, int line) {
    const unsigned char bg  = 6;   // (6=light_blue,1=red,4=darkblue,9=light_red)
    const unsigned char card_fill = 0;    // black
    const unsigned char text  = 15;       // white
    const unsigned char subtext = 14;     // yellow

    clearScreen(bg);

    for (volatile int i = 0; i < 100000000; i++);

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
    drawStringSized(100, 960, (char*)"emexOS rpi4 - system halted.", 15, 24);

    //for (;;) { }
}
