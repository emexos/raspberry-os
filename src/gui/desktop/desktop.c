#include "../../include/io.h"
#include "../../include/fb.h"
#include "../../include/desktop.h"

void desktop() {
    drawStringSized(90, 50, "emexos rpi", 0x0f, 16);
    drawString(100,100,"Hello world!",0x0f);
    drawString(100,110,"Shell loaded",0x0f);
    drawString(100,120,"Mouse driver active",0x0e);

    // Draw bottom bar
    drawRect(0,1040,1921,1080,0xFF,1);

    // "Click me!" Button
    drawRect(300,90,520,150,0xcc,1);
    drawString(320,100,"Click me!",0xcf);

    while (1) {
        for (volatile int i = 0; i < 10000; i++);
    }
}
