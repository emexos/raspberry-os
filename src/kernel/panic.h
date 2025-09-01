#pragma once

void kernel_panic_screen(const char* title, const char* reason, const char* file, int line) __attribute__((noreturn));
void drawEmexLogo();

#define PANIC(msg) kernel_panic_screen("KERNEL PANIC", (msg), __FILE__, __LINE__)
#define ASSERT_OK(expr) do { if (!(expr)) kernel_panic_screen("KERNEL PANIC", "Assertion failed: " #expr, __FILE__, __LINE__); } while (0)
