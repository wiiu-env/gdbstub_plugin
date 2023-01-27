
#include "screen.h"
#include "logger.h"
#include <coreinit/debug.h>
#include <gx2/state.h>

#include <memory/mappedmemory.h>

Screen::Screen() : screenBuffer(nullptr) {
    init();
}
Screen::~Screen() {
    destroyBuffer();
}

void Screen::init() {
    OSScreenInit();

    uint32_t bufferSize0 = OSScreenGetBufferSizeEx(SCREEN_TV);
    uint32_t bufferSize1 = OSScreenGetBufferSizeEx(SCREEN_DRC);
    screenBuffer         = MEMAllocFromMappedMemoryForGX2Ex(bufferSize0 + bufferSize1, 0x100);

    if (screenBuffer == nullptr) {
        OSFatal("Failed to allocate screenbuffer");
    }
    memset(screenBuffer, 0, bufferSize0 + bufferSize1);
    OSScreenSetBufferEx(SCREEN_TV, screenBuffer);
    OSScreenSetBufferEx(SCREEN_DRC, (char *) screenBuffer + bufferSize0);

    OSScreenEnableEx(SCREEN_TV, 1);
    OSScreenEnableEx(SCREEN_DRC, 1);
    OSScreenClearBufferEx(SCREEN_TV, 0);
    OSScreenClearBufferEx(SCREEN_DRC, 0);
    OSScreenFlipBuffersEx(SCREEN_TV);
    OSScreenFlipBuffersEx(SCREEN_DRC);
}

void Screen::clear(OSScreenID screen, uint32_t color) {
    OSScreenClearBufferEx(screen, color);
}

void Screen::drawRect(OSScreenID screen, int x1, int y1, int x2, int y2, uint32_t color) {
    for (int x = x1; x < x2; x++) {
        OSScreenPutPixelEx(screen, x, y1, color);
        OSScreenPutPixelEx(screen, x, y2, color);
    }
    for (int y = y1; y < y2; y++) {
        OSScreenPutPixelEx(screen, x1, y, color);
        OSScreenPutPixelEx(screen, x2, y, color);
    }
}

void Screen::fillRect(OSScreenID screen, int x1, int y1, int x2, int y2, uint32_t color) {
    for (int x = x1; x < x2; x++) {
        for (int y = y1; y < y2; y++) {
            OSScreenPutPixelEx(screen, x, y, color);
        }
    }
}

void Screen::drawText(OSScreenID screen, int x, int y, const char *text) {
    OSScreenPutFontEx(screen, x, y, text);
}

void Screen::flip(OSScreenID screen) {
    OSScreenFlipBuffersEx(screen);
}

int Screen::convx(int x) { return x * 854 / 1280; }
int Screen::convy(int y) { return y * 480 / 720; }

void Screen::clear(uint32_t color) {
    clear(SCREEN_TV, color);
    clear(SCREEN_DRC, color);
}

void Screen::drawRect(int x1, int y1, int x2, int y2, uint32_t color) {
    drawRect(SCREEN_TV, x1, y1, x2, y2, color);
    drawRect(SCREEN_DRC, convx(x1), convy(y1), convx(x2), convy(y2), color);
}

void Screen::fillRect(int x1, int y1, int x2, int y2, uint32_t color) {
    fillRect(SCREEN_TV, x1, y1, x2, y2, color);
    fillRect(SCREEN_DRC, convx(x1), convy(y1), convx(x2), convy(y2), color);
}

void Screen::drawText(int x, int y, const char *text) {
    drawText(SCREEN_TV, x, y, text);
    drawText(SCREEN_DRC, x, y, text);
}

void Screen::flip() {
    flip(SCREEN_TV);
    flip(SCREEN_DRC);
}
void Screen::destroyBuffer() {
    if (screenBuffer) {
        MEMFreeToMappedMemory(screenBuffer);
        screenBuffer = nullptr;
    }

    GX2Init(nullptr);
}
