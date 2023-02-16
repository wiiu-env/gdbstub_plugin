#include "IOUtils.h"
#include <cstdint>
#include <cstring>

#define THREAD_INFO_BUFFER_CAPACITY 0x10001
static char sThreadInfoBuffer[THREAD_INFO_BUFFER_CAPACITY] = {};
static uint32_t sThreadInfoBufferOffset                    = 0;

char *GetThreadInfoBuffer() {
    sThreadInfoBuffer[sThreadInfoBufferOffset] = '\0';
    return sThreadInfoBuffer;
}
void InitThreadInfoBuffer() {
    memset(sThreadInfoBuffer, 0, sizeof(sThreadInfoBuffer));
    sThreadInfoBufferOffset = 0;
}

bool PutStringIntoThreadInfoBuffer(const char *str) {
    uint32_t offset = 0;
    while (str[offset] != '\0') {
        if (sThreadInfoBufferOffset < (THREAD_INFO_BUFFER_CAPACITY - 2)) {
            sThreadInfoBuffer[sThreadInfoBufferOffset] = str[offset];
            sThreadInfoBufferOffset++;
        } else {
            return false;
        }
        offset++;
    }
    return true;
}

bool PutCharIntoThreadInfoBuffer(char c) {
    // we want to keep a null byte at the end
    if (sThreadInfoBufferOffset < (THREAD_INFO_BUFFER_CAPACITY - 2)) {
        sThreadInfoBuffer[sThreadInfoBufferOffset] = c;
        sThreadInfoBufferOffset++;
    } else {
        return false;
    }
    return true;
}

void PutXMLEscapedStringIntoThreadInfoBuffer(const char *str) {
    if (str != nullptr) {
        const char *curPtr = str;
        while (curPtr[0] != '\0') {
            switch (curPtr[0]) {
                case '<':
                    PutStringIntoThreadInfoBuffer("&lt;");
                    break;
                case '>':
                    PutStringIntoThreadInfoBuffer("&gt;");
                    break;
                case '&':
                    PutStringIntoThreadInfoBuffer("&amp;");
                    break;
                case '"':
                    PutStringIntoThreadInfoBuffer("&quot;");
                    break;
                case '\'':
                    PutStringIntoThreadInfoBuffer("&apos;");
                    break;
                default:
                    PutCharIntoThreadInfoBuffer(curPtr[0]);
                    break;
            }
            curPtr++;
        }
    }
}
