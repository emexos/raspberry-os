#include "../include/io.h"
#include "../include/mb.h"
#include "../include/font/terminal.h"

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

unsigned int width, height, pitch, isrgb;
unsigned char *fb;

void fb_init()
{
    mbox[0] = 35*4; // Length of message in bytes
    mbox[1] = MBOX_REQUEST;

    mbox[2] = MBOX_TAG_SETPHYWH; // Tag identifier
    mbox[3] = 8; // Value size in bytes
    mbox[4] = 0;
    mbox[5] = 1920; // Value(width)
    mbox[6] = 1080; // Value(height)

    mbox[7] = MBOX_TAG_SETVIRTWH;
    mbox[8] = 8;
    mbox[9] = 8;
    mbox[10] = 1920;
    mbox[11] = 1080;

    mbox[12] = MBOX_TAG_SETVIRTOFF;
    mbox[13] = 8;
    mbox[14] = 8;
    mbox[15] = 0; // Value(x)
    mbox[16] = 0; // Value(y)

    mbox[17] = MBOX_TAG_SETDEPTH;
    mbox[18] = 4;
    mbox[19] = 4;
    mbox[20] = 32; // Bits per pixel

    mbox[21] = MBOX_TAG_SETPXLORDR;
    mbox[22] = 4;
    mbox[23] = 4;
    mbox[24] = 1; // RGB

    mbox[25] = MBOX_TAG_GETFB;
    mbox[26] = 8;
    mbox[27] = 8;
    mbox[28] = 4096; // FrameBufferInfo.pointer
    mbox[29] = 0;    // FrameBufferInfo.size

    mbox[30] = MBOX_TAG_GETPITCH;
    mbox[31] = 4;
    mbox[32] = 4;
    mbox[33] = 0; // Bytes per line

    mbox[34] = MBOX_TAG_LAST;

    // Check call is successful and we have a pointer with depth 32
    if (mbox_call(MBOX_CH_PROP) && mbox[20] == 32 && mbox[28] != 0) {
        mbox[28] &= 0x3FFFFFFF; // Convert GPU address to ARM address
        width = mbox[10];       // Actual physical width
        height = mbox[11];      // Actual physical height
        pitch = mbox[33];       // Number of bytes per line
        isrgb = mbox[24];       // Pixel order
        fb = (unsigned char *)((long)mbox[28]);
    }

}

int getFontPixel(char c, int x, int y) {
    unsigned char uc = (unsigned char)c;

    // dein Font startet bei ASCII 32 (Space)
    if (uc < 32 || uc >= 32 + FONT_NUMGLYPHS) {
        return 0; // unbekanntes Zeichen → leer
    }

    int glyphIndex = uc - 32; // ab Space (32)
    unsigned char line = font[glyphIndex][y];
    return (line >> x) & 1;
}

void drawPixel(int x, int y, unsigned char attr)
{
    int offs = (y * pitch) + (x * 4);
    *((unsigned int*)(fb + offs)) = vgapal[attr & 0x0f];
}

void clearScreen(unsigned char color) {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            drawPixel(x, y, color);
        }
    }
}

void drawRect(int x1, int y1, int x2, int y2, unsigned char attr, int fill)
{
    int y=y1;

    while (y <= y2) {
       int x=x1;
       while (x <= x2) {
	  if ((x == x1 || x == x2) || (y == y1 || y == y2)) drawPixel(x, y, attr);
	  else if (fill) drawPixel(x, y, (attr & 0xf0) >> 4);
          x++;
       }
       y++;
    }
}

void drawRoundedRect(int x1, int y1, int x2, int y2, int radius,
                     unsigned char fillAttr, int fill,
                     unsigned char borderAttr, int borderThickness) {
    if (borderAttr < 0) borderAttr = fillAttr; // Default: Rand = Füllung
    if (borderThickness < 1) borderThickness = 1;

    for (int y = y1; y <= y2; y++) {
        for (int x = x1; x <= x2; x++) {
            int inCorner = 0;
            int isBorder = 0;

            // check corners
            if (x < x1 + radius && y < y1 + radius) { // oben links
                int dx = x - (x1 + radius);
                int dy = y - (y1 + radius);
                int dist2 = dx*dx + dy*dy;
                if (dist2 > radius*radius) continue; // außerhalb
                if (dist2 >= (radius-borderThickness)*(radius-borderThickness)) isBorder = 1;
            } else if (x > x2 - radius && y < y1 + radius) { // oben rechts
                int dx = x - (x2 - radius);
                int dy = y - (y1 + radius);
                int dist2 = dx*dx + dy*dy;
                if (dist2 > radius*radius) continue;
                if (dist2 >= (radius-borderThickness)*(radius-borderThickness)) isBorder = 1;
            } else if (x < x1 + radius && y > y2 - radius) { // unten links
                int dx = x - (x1 + radius);
                int dy = y - (y2 - radius);
                int dist2 = dx*dx + dy*dy;
                if (dist2 > radius*radius) continue;
                if (dist2 >= (radius-borderThickness)*(radius-borderThickness)) isBorder = 1;
            } else if (x > x2 - radius && y > y2 - radius) { // unten rechts
                int dx = x - (x2 - radius);
                int dy = y - (y2 - radius);
                int dist2 = dx*dx + dy*dy;
                if (dist2 > radius*radius) continue;
                if (dist2 >= (radius-borderThickness)*(radius-borderThickness)) isBorder = 1;
            } else {
                // gerader Teil
                if ((x < x1 + borderThickness) || (x > x2 - borderThickness) ||
                    (y < y1 + borderThickness) || (y > y2 - borderThickness)) {
                    isBorder = 1;
                }
            }

            if (isBorder) {
                drawPixel(x, y, borderAttr);
            } else if (fill) {
                drawPixel(x, y, (fillAttr & 0xf0) >> 4);
            }
        }
    }
}

void drawLine(int x1, int y1, int x2, int y2, unsigned char attr)
{
    int dx, dy, p, x, y;

    dx = x2-x1;
    dy = y2-y1;
    x = x1;
    y = y1;
    p = 2*dy-dx;

    while (x<x2) {
       if (p >= 0) {
          drawPixel(x,y,attr);
          y++;
          p = p+2*dy-2*dx;
       } else {
          drawPixel(x,y,attr);
          p = p+2*dy;
       }
       x++;
    }
}

void drawCircle(int x0, int y0, int radius, unsigned char attr, int fill)
{
    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y) {
	if (fill) {
	   drawLine(x0 - y, y0 + x, x0 + y, y0 + x, (attr & 0xf0) >> 4);
	   drawLine(x0 - x, y0 + y, x0 + x, y0 + y, (attr & 0xf0) >> 4);
	   drawLine(x0 - x, y0 - y, x0 + x, y0 - y, (attr & 0xf0) >> 4);
	   drawLine(x0 - y, y0 - x, x0 + y, y0 - x, (attr & 0xf0) >> 4);
	}
	drawPixel(x0 - y, y0 + x, attr);
	drawPixel(x0 + y, y0 + x, attr);
	drawPixel(x0 - x, y0 + y, attr);
        drawPixel(x0 + x, y0 + y, attr);
	drawPixel(x0 - x, y0 - y, attr);
	drawPixel(x0 + x, y0 - y, attr);
	drawPixel(x0 - y, y0 - x, attr);
	drawPixel(x0 + y, y0 - x, attr);

	if (err <= 0) {
	    y += 1;
	    err += 2*y + 1;
	}

	if (err > 0) {
	    x -= 1;
	    err -= 2*x + 1;
	}
    }
}

void drawChar(unsigned char ch, int x, int y, unsigned char attr)
{
    unsigned char *glyph = (unsigned char *)&font + (ch < FONT_NUMGLYPHS ? ch : 0) * FONT_BPG;

    for (int i=0;i<FONT_HEIGHT;i++) {
	for (int j=0;j<FONT_WIDTH;j++) {
	    unsigned char mask = 1 << j;
	    unsigned char col = (*glyph & mask) ? attr & 0x0f : (attr & 0xf0) >> 4;

	    drawPixel(x+j, y+i, col);
	}
	glyph += FONT_BPL;
    }
}

void drawCharSized(unsigned char ch, int x, int y, unsigned char attr, int size)
{
    unsigned char *glyph = (unsigned char *)&font + (ch < FONT_NUMGLYPHS ? ch : 0) * FONT_BPG;
    int scale = size / FONT_WIDTH;

    for (int dy = 0; dy < FONT_HEIGHT; dy++) {
        for (int dx = 0; dx < FONT_WIDTH; dx++) {
            unsigned char mask = 1 << dx;
            if (*glyph & mask) {
                for (int sy = 0; sy < scale; sy++) {
                    for (int sx = 0; sx < scale; sx++) {
                        drawPixel(x + dx*scale + sx, y + dy*scale + sy, attr & 0x0f);
                    }
                }
            } else {
                for (int sy = 0; sy < scale; sy++) {
                    for (int sx = 0; sx < scale; sx++) {
                        drawPixel(x + dx*scale + sx, y + dy*scale + sy, (attr & 0xf0) >> 4);
                    }
                }
            }
        }
        glyph += FONT_BPL;
    }
}


void drawString(int x, int y, char *s, unsigned char attr)
{
    while (*s) {
       if (*s == '\r') {
          x = 0;
       } else if(*s == '\n') {
          x = 0; y += FONT_HEIGHT;
       } else {
	  drawChar(*s, x, y, attr);
          x += FONT_WIDTH;
       }
       s++;
    }
}

void drawStringSized(int x, int y, char *s, unsigned char attr, int size)
{
    int scale = size / FONT_WIDTH;

    while (*s) {
        if (*s == '\r') {
            x = 0;
        } else if (*s == '\n') {
            x = 0; y += FONT_HEIGHT * scale;
        } else {
            drawCharSized(*s, x, y, attr, size);
            x += FONT_WIDTH * scale;
        }
        s++;
    }
}
