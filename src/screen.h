
#pragma once

#include <coreinit/screen.h>
#include <cstdint>

class Screen {
public:
    Screen();
    ~Screen();

    void init();
    void clear(uint32_t color);
    void drawRect(int x1, int y1, int x2, int y2, uint32_t color);
    void fillRect(int x1, int y1, int x2, int y2, uint32_t color);
    static void drawText(int x, int y, const char *text);
    static void flip();

    static void clear(OSScreenID screen, uint32_t color);
    static void drawRect(OSScreenID screen, int x1, int y1, int x2, int y2, uint32_t color);
    static void fillRect(OSScreenID screen, int x1, int y1, int x2, int y2, uint32_t color);
    static void drawText(OSScreenID screen, int x, int y, const char *text);
    static void flip(OSScreenID screen);

    void destroyBuffer();

private:
    void *screenBuffer;

    static int convx(int x);
    static int convy(int y);
};
