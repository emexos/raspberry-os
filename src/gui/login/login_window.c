#include "../../include/io.h"
#include "../../include/fb.h"
#include "../../include/login_window.h"

void login() {
    drawRect(0, 0, 1920, 1080, 0x88, 1); // gray

    // Title
    drawStringSized(395, 305, "emexOS login screen", 0x80, 64); //shadow
    drawStringSized(400, 300, "emexOS login screen", 0x09, 64);

    // Login Password Input Box
    drawRoundedRect(710, 800, 1210, 880, 42, 0x77, 1, 0x77, 3);

    // input box logic will be here
}
