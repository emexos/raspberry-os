#ifndef FB_H
#define FB_H

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

void fb_init();
void drawPixel(int x, int y, unsigned char attr);
void drawChar(unsigned char ch, int x, int y, unsigned char attr);
void drawString(int x, int y, char *s, unsigned char attr);
void drawRect(int x1, int y1, int x2, int y2, unsigned char attr, int fill);
void drawCircle(int x0, int y0, int radius, unsigned char attr, int fill);
void drawLine(int x1, int y1, int x2, int y2, unsigned char attr);
void drawRoundedRect(int x1, int y1, int x2, int y2, int radius,
                     unsigned char fillAttr, int fill,
                     unsigned char borderAttr, int borderThickness);

void drawCharSized(char c, int x, int y, unsigned char attr, int scale);
void drawStringSized(int x, int y, char *s, unsigned char attr, int scale);
int getFontPixel(char c, int x, int y);
void clearScreen(unsigned char color);

#endif
