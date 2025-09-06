#include "../../include/io.h"
#include "../../include/fb.h"
#include "../../include/desktop.h"

void desktop() {
    clearScreen(0x00);

    drawStringSized(90, 50, "emexOS rpi4", 0x0f, 16);

    drawRect(90,90,320,150,0xcc,1);
    drawString(110,100,"This is a message box",0xcf);
    drawString(110,110,"for testing graphics...",0xcf);

    while (1) {
        for (volatile int i = 0; i < 10000; i++);
    }
}
