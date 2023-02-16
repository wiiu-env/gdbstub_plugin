#include "GDB_IO.h"
#include "GDBUtils.h"
#include "imports.h"

#include <coreinit/core.h>
#include <coreinit/debug.h>
#include <cstring>
#include <wups/function_patching.h>

static const char *sHexChars          = "0123456789abcdef";
static int32_t gIOOutputOffset        = 0;
int32_t gGDBCharsInIOBuffer           = 0;
int32_t gGDBCharsOfIOBufferConsumed   = 0;
char gIOBuffer[IO_BUFFER_CAPACITY]    = {};
char gIOOutBuffer[IO_BUFFER_CAPACITY] = {};

static char sGDBCharUndoGetBuffer = '\0';

void ResetIOOffsets() {
    gIOOutputOffset             = 0;
    gGDBCharsInIOBuffer         = 0;
    gGDBCharsOfIOBufferConsumed = 0;
    memset(gIOBuffer, 0, sizeof(gIOBuffer));
    memset(gIOOutBuffer, 0, sizeof(gIOOutBuffer));
    sGDBCharUndoGetBuffer = '\0';
}

// 0x02039158
DECL_FUNCTION(void, MasterAgent_IOInit) {
    gIOOutputOffset = 0;
}

// 0x0203a76c
DECL_FUNCTION(int32_t, MasterAgent_IOCurrentIndex) {
    return gIOOutputOffset;
}

// 0x0203a778
DECL_FUNCTION(int32_t, MasterAgent_IORemaining) {
    return (IO_BUFFER_CAPACITY - 4) - gIOOutputOffset;
}

// 0x0203a788
DECL_FUNCTION(void, MasterAgent_IORestoreIndex, int32_t index) {
    gIOOutputOffset = index;
}

// 0x0203a758
DECL_FUNCTION(char *, MasterAgent_IOCurrentBufferPointer) {
    return &gIOOutBuffer[gIOOutputOffset];
}

// 0x02039168
DECL_FUNCTION(void, MasterAgent_IOPutChar, char curChar) {
    if ((IO_BUFFER_CAPACITY - 5) < gIOOutputOffset) {
        return;
    }

    gIOOutBuffer[gIOOutputOffset] = curChar;
    gIOOutputOffset++;
}

// 0x0203a7ec
DECL_FUNCTION(void, MasterAgent_IOPutHex4, uint8_t val) {
    if ((IO_BUFFER_CAPACITY - 5) < gIOOutputOffset) {
        return;
    }
    gIOOutBuffer[gIOOutputOffset] = sHexChars[val & 0xf];
    gIOOutputOffset++;
}

// 0x0203a794
DECL_FUNCTION(void, MasterAgent_IOPutHex8, uint8_t val) {
    if (gIOOutputOffset < (IO_BUFFER_CAPACITY - 4 - 1)) {
        gIOOutBuffer[gIOOutputOffset]     = sHexChars[(val >> 4) & 0xf];
        gIOOutBuffer[gIOOutputOffset + 1] = sHexChars[val & 0xf];
        gIOOutputOffset += 2;
    } else {
        if (gIOOutputOffset < (IO_BUFFER_CAPACITY - 4)) {
            gIOOutBuffer[gIOOutputOffset] = 'f';
        }
    }
}

// 0x0203a884
DECL_FUNCTION(void, MasterAgent_IOPutHex16, uint16_t value) {
    if (gIOOutputOffset < (IO_BUFFER_CAPACITY - 4 - 3)) {
        gIOOutBuffer[gIOOutputOffset]     = sHexChars[(value >> 0xc) & 0xf];
        gIOOutBuffer[gIOOutputOffset + 1] = sHexChars[(value >> 0x8) & 0xf];
        gIOOutBuffer[gIOOutputOffset + 2] = sHexChars[(value >> 0x4) & 0xf];
        gIOOutBuffer[gIOOutputOffset + 3] = sHexChars[(value >> 0x0) & 0xf];
        gIOOutputOffset += 4;
    } else {
        auto offsetCpy = gIOOutputOffset;
        while (offsetCpy < (IO_BUFFER_CAPACITY - 4)) {
            gIOOutBuffer[offsetCpy] = 'f';
            offsetCpy++;
        }
    }
}

// 0x02039190
DECL_FUNCTION(void, MasterAgent_IOPutHex32, uint32_t value) {
    if (gIOOutputOffset < (IO_BUFFER_CAPACITY - 4 - 7)) {
        gIOOutBuffer[gIOOutputOffset]     = sHexChars[value >> 0x1c & 0xf];
        gIOOutBuffer[gIOOutputOffset + 1] = sHexChars[value >> 0x18 & 0xf];
        gIOOutBuffer[gIOOutputOffset + 2] = sHexChars[value >> 0x14 & 0xf];
        gIOOutBuffer[gIOOutputOffset + 3] = sHexChars[value >> 0x10 & 0xf];
        gIOOutBuffer[gIOOutputOffset + 4] = sHexChars[value >> 0xc & 0xf];
        gIOOutBuffer[gIOOutputOffset + 5] = sHexChars[value >> 0x8 & 0xf];
        gIOOutBuffer[gIOOutputOffset + 6] = sHexChars[value >> 0x4 & 0xf];
        gIOOutBuffer[gIOOutputOffset + 7] = sHexChars[value >> 0x0 & 0xf];
        gIOOutputOffset += 8;
    } else {
        auto offsetCpy = gIOOutputOffset;
        while (offsetCpy < (IO_BUFFER_CAPACITY - 4)) {
            gIOOutBuffer[offsetCpy] = 'f';
            offsetCpy++;
        }
    }
}

// 0x0203a81c
DECL_FUNCTION(void, MasterAgent_IOPutStringAsHex, const char *str) {
    uint32_t offset = 0;
    char curChar    = *str;
    while (curChar != 0) {
        if (gIOOutputOffset < (IO_BUFFER_CAPACITY - 4 - 1)) {
            gIOOutBuffer[gIOOutputOffset]     = sHexChars[curChar >> 4];
            gIOOutBuffer[gIOOutputOffset + 1] = sHexChars[curChar & 0xf];
            gIOOutputOffset += 2;
        }
        offset++;
        curChar = str[offset];
    }
}

void my_MasterAgent_PutPacket(const char *packetStr, bool checkPendingInputFirst);

DECL_FUNCTION(void, MasterAgent_IOSendEx, bool unknwn1) {
    if ((IO_BUFFER_CAPACITY - 4) < gIOOutputOffset) {
        return;
    }
    gIOOutBuffer[gIOOutputOffset] = '\0';
    gIOOutputOffset++;
    my_MasterAgent_PutPacket(gIOOutBuffer, unknwn1);
}

// Checked
DECL_FUNCTION(void, MasterAgent_IOPutString, const char *str) {
    uint32_t offset = 0;
    auto curChar    = str[0];
    while (curChar != '\0') {
        auto uVar1 = gIOOutputOffset;
        if (gIOOutputOffset < (IO_BUFFER_CAPACITY - 4)) {
            uVar1                         = gIOOutputOffset + 1;
            gIOOutBuffer[gIOOutputOffset] = curChar;
        }
        offset          = offset + 1;
        gIOOutputOffset = uVar1;
        curChar         = str[offset];
    }
}

extern "C" int32_t DK_PCharReadAsync(uint32_t channel_guess, uint32_t bufferSize, char *buffer);

// 0x02039250
DECL_FUNCTION(bool, MasterAgent_IOPendingInput) {
    auto masterCore = GDBStub_MasterCore();
    if (OSGetCoreId() == masterCore) {
        if (gIsDebuggerInitializedOnCore[masterCore] != 0) {
            if ((__OSGetAppFlags() & 0x80) == 0) {
                gGDBCharsInIOBuffer = Waikiki_Read(gIOBuffer, IO_BUFFER_CAPACITY);
            } else if (gGDBCharsInIOBuffer == gGDBCharsOfIOBufferConsumed) {
                gGDBCharsInIOBuffer = DK_PCharReadAsync(1, IO_BUFFER_CAPACITY, gIOBuffer);
            }
            gGDBCharsOfIOBufferConsumed = 0;
            if (gGDBCharsInIOBuffer < 1) {
                gGDBCharsInIOBuffer = 0;
                return false;
            }
            return true;
        }
    }
    return false;
}

// Checked
DECL_FUNCTION(void, MasterAgent_IOUnGetChar, char charToUndo) {
    sGDBCharUndoGetBuffer = charToUndo;
}

// 0x0203936c
DECL_FUNCTION(char, MasterAgent_IOGetChar) {
    char result;

    result = sGDBCharUndoGetBuffer;
    if (sGDBCharUndoGetBuffer != '\0') {
        sGDBCharUndoGetBuffer = '\0';
        return result;
    }
    if (gGDBCharsInIOBuffer <= gGDBCharsOfIOBufferConsumed) {
        while (!my_MasterAgent_IOPendingInput()) {
            // wait for input...
        }
        if (gGDBCharsInIOBuffer <= gGDBCharsOfIOBufferConsumed) {
            return -1;
        }
    }
    auto offsetCpy = gGDBCharsOfIOBufferConsumed;
    gGDBCharsOfIOBufferConsumed++;
    return gIOBuffer[offsetCpy];
}

// Doubled capacity to be able to escape every single char, add 4 for the '$' prefix and '#00' checksum suffix
static char sWritePacketBuffer[(IO_BUFFER_CAPACITY * 2) + 4];

DECL_FUNCTION(void, MasterAgent_PutPacket, const char *packetStr, bool checkPendingInputFirst) {
#ifdef PACKET_LOGGING
    OSReport("Send packet \"%s\"\n", packetStr);
#endif
    uint32_t tryCount      = 0;
    bool ctrl_c_seen       = false;
    bool checkPendingInput = checkPendingInputFirst;
    while (true) {
        if (checkPendingInput) {
            while (my_MasterAgent_IOPendingInput()) {
                auto curChar = my_MasterAgent_IOGetChar();
                if (curChar == '$') {
                    my_MasterAgent_IOUnGetChar((char) curChar);
                    return;
                }
                if (curChar == '\x03') {
                    ctrl_c_seen = true;
                }
            }
        }

        if (packetStr == nullptr) {
            MasterAgent_IOWriteString("$#00");
        } else {
            memset(sWritePacketBuffer, 0, sizeof(sWritePacketBuffer));
            auto *outBufferPtr    = sWritePacketBuffer;
            sWritePacketBuffer[0] = '$';
            outBufferPtr++;
            uint32_t checksum = 0;
            uint32_t offset   = 0;
            auto curChar      = packetStr[offset];
            while (curChar != '\0') {
                if ((curChar == '#' || curChar == '$' || curChar == '}' || curChar == '*')) {
                    outBufferPtr[0] = '}';
                    checksum += '}';
                    outBufferPtr[1] = (char) (curChar ^ 0x20);
                    checksum += (char) (curChar ^ 0x20);
                    outBufferPtr++;
                } else {
                    outBufferPtr[0] = curChar;
                    checksum += curChar;
                }
                offset++;
                outBufferPtr++;
                curChar = packetStr[offset];
            }

            outBufferPtr[0] = '#';
            outBufferPtr[1] = sHexChars[(checksum & 0xff) >> 4];
            outBufferPtr[2] = sHexChars[checksum & 0xf];
            MasterAgent_IOWriteString(sWritePacketBuffer);
        }

        do {
            auto curChar = my_MasterAgent_IOGetChar();
            if (((curChar == '+') || (curChar == '-')) || (curChar == '$')) {
                if (ctrl_c_seen) {
                    gCoreData[GDBStub_MasterCore()].signal = GDBSIGNAL_SIGINT;
                }
                if (curChar == '+') {
                    return;
                }
                if (curChar == '-') {
                    tryCount++;
                    if (tryCount > 9) {
                        OSReport("MasterAgent_PutPacket: NACK received 10 times in a row\n");
                        return;
                    }
                    checkPendingInput = true;
                    break;
                } else {
                    my_MasterAgent_IOUnGetChar((char) curChar);
                    return;
                }
            }
            if (curChar == '\x03') {
                ctrl_c_seen = true;
            }
        } while (!my_MasterAgent_IOPendingInput());
    }
}

DECL_FUNCTION(void, MasterAgent_SendResult, uint32_t result) {
    if (result == 0) {
        MasterAgent_PutPacket("OK", false);
        return;
    }
    MasterAgent_IOInit();
    my_MasterAgent_IOPutChar(0x45);
    my_MasterAgent_IOPutHex8((uint8_t) (result & 0xFF));
    MasterAgent_IOSendEx(false);
}

WUPS_MUST_REPLACE_PHYSICAL_FOR_PROCESS(MasterAgent_IOInit, (0x3201C400 + (0x02039158 - 0x02000000)), (0x101C400 + (0x02039158 - 0x02000000)), WUPS_FP_TARGET_PROCESS_GAME_AND_MENU);
WUPS_MUST_REPLACE_PHYSICAL_FOR_PROCESS(MasterAgent_IOCurrentIndex, (0x3201C400 + (0x0203a76c - 0x02000000)), (0x101C400 + (0x0203a76c - 0x02000000)), WUPS_FP_TARGET_PROCESS_GAME_AND_MENU);
WUPS_MUST_REPLACE_PHYSICAL_FOR_PROCESS(MasterAgent_IORemaining, (0x3201C400 + (0x0203a778 - 0x02000000)), (0x101C400 + (0x0203a778 - 0x02000000)), WUPS_FP_TARGET_PROCESS_GAME_AND_MENU);
WUPS_MUST_REPLACE_PHYSICAL_FOR_PROCESS(MasterAgent_IORestoreIndex, (0x3201C400 + (0x0203a788 - 0x02000000)), (0x101C400 + (0x0203a788 - 0x02000000)), WUPS_FP_TARGET_PROCESS_GAME_AND_MENU);
WUPS_MUST_REPLACE_PHYSICAL_FOR_PROCESS(MasterAgent_IOCurrentBufferPointer, (0x3201C400 + (0x0203a758 - 0x02000000)), (0x101C400 + (0x0203a758 - 0x02000000)), WUPS_FP_TARGET_PROCESS_GAME_AND_MENU);
WUPS_MUST_REPLACE_PHYSICAL_FOR_PROCESS(MasterAgent_IOPutChar, (0x3201C400 + (0x02039168 - 0x02000000)), (0x101C400 + (0x02039168 - 0x02000000)), WUPS_FP_TARGET_PROCESS_GAME_AND_MENU);
WUPS_MUST_REPLACE_PHYSICAL_FOR_PROCESS(MasterAgent_IOPutHex4, (0x3201C400 + (0x0203a7ec - 0x02000000)), (0x101C400 + (0x0203a7ec - 0x02000000)), WUPS_FP_TARGET_PROCESS_GAME_AND_MENU);
WUPS_MUST_REPLACE_PHYSICAL_FOR_PROCESS(MasterAgent_IOPutHex8, (0x3201C400 + (0x0203a794 - 0x02000000)), (0x101C400 + (0x0203a794 - 0x02000000)), WUPS_FP_TARGET_PROCESS_GAME_AND_MENU);
WUPS_MUST_REPLACE_PHYSICAL_FOR_PROCESS(MasterAgent_IOPutHex16, (0x3201C400 + (0x0203a884 - 0x02000000)), (0x101C400 + (0x0203a884 - 0x02000000)), WUPS_FP_TARGET_PROCESS_GAME_AND_MENU);
WUPS_MUST_REPLACE_PHYSICAL_FOR_PROCESS(MasterAgent_IOPutHex32, (0x3201C400 + (0x02039190 - 0x02000000)), (0x101C400 + (0x02039190 - 0x02000000)), WUPS_FP_TARGET_PROCESS_GAME_AND_MENU);
WUPS_MUST_REPLACE_PHYSICAL_FOR_PROCESS(MasterAgent_IOPutStringAsHex, (0x3201C400 + (0x0203a81c - 0x02000000)), (0x101C400 + (0x0203a81c - 0x02000000)), WUPS_FP_TARGET_PROCESS_GAME_AND_MENU);
WUPS_MUST_REPLACE_PHYSICAL_FOR_PROCESS(MasterAgent_IOSendEx, (0x3201C400 + (0x02039a28 - 0x02000000)), (0x101C400 + (0x02039a28 - 0x02000000)), WUPS_FP_TARGET_PROCESS_GAME_AND_MENU);
WUPS_MUST_REPLACE_PHYSICAL_FOR_PROCESS(MasterAgent_IOPutString, (0x3201C400 + (0x0203b6b4 - 0x02000000)), (0x101C400 + (0x0203b6b4 - 0x02000000)), WUPS_FP_TARGET_PROCESS_GAME_AND_MENU);
WUPS_MUST_REPLACE_PHYSICAL_FOR_PROCESS(MasterAgent_IOPendingInput, (0x3201C400 + (0x02039250 - 0x02000000)), (0x101C400 + (0x02039250 - 0x02000000)), WUPS_FP_TARGET_PROCESS_GAME_AND_MENU);
WUPS_MUST_REPLACE_PHYSICAL_FOR_PROCESS(MasterAgent_IOUnGetChar, (0x3201C400 + (0x0203942c - 0x02000000)), (0x101C400 + (0x0203942c - 0x02000000)), WUPS_FP_TARGET_PROCESS_GAME_AND_MENU);
WUPS_MUST_REPLACE_PHYSICAL_FOR_PROCESS(MasterAgent_IOGetChar, (0x3201C400 + (0x0203936c - 0x02000000)), (0x101C400 + (0x0203936c - 0x02000000)), WUPS_FP_TARGET_PROCESS_GAME_AND_MENU);
WUPS_MUST_REPLACE_PHYSICAL_FOR_PROCESS(MasterAgent_PutPacket, (0x3201C400 + (0x020394d8 - 0x02000000)), (0x101C400 + (0x020394d8 - 0x02000000)), WUPS_FP_TARGET_PROCESS_GAME_AND_MENU);
WUPS_MUST_REPLACE_PHYSICAL_FOR_PROCESS(MasterAgent_SendResult, (0x3201C400 + (0x0203bafc - 0x02000000)), (0x101C400 + (0x0203bafc - 0x02000000)), WUPS_FP_TARGET_PROCESS_GAME_AND_MENU);