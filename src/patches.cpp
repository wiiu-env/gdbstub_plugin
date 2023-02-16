#include "global_vars.h"
#include "imports.h"
#include <cstring>
#include <wups/function_patching.h>

DECL_FUNCTION(void, OSPerGameInits) {
    real_OSPerGameInits();
    if (gInitStubs && !gHeartbeatDisabled) {
        OSDisableUserHeartbeat();
        gHeartbeatDisabled = true;
    }
}

// Hooks into an empty function after the appflags were already set.
DECL_FUNCTION(void, EntryHook) {
    if (gInitStubs) {
        gAppflags |= 0x01; // Enable the GDBStub!
        gAppflags |= 0x80; // We want to use PChar instead of EXI
    }
}

// We have to "hide" the "GDBStub enabled" flag for everything that's using __OSGetAppFlags
DECL_FUNCTION(uint32_t, __OSGetAppFlags) {
    auto res = real___OSGetAppFlags();
    if (gInitStubs) {
        res = res & ~0x01;
    }
    return res;
}

DECL_FUNCTION(int32_t, makeOpenPath, const char *path, char *out) {
    strncpy(out, "/dev/iosuhax", 0x20);
    return 0;
}

WUPS_MUST_REPLACE_PHYSICAL_FOR_PROCESS(makeOpenPath, (0x3201C400 + (0x02040390 - 0x02000000)), (0x101C400 + (0x02040390 - 0x02000000)), WUPS_FP_TARGET_PROCESS_ALL);
WUPS_MUST_REPLACE_PHYSICAL_FOR_PROCESS(OSPerGameInits, (0x3001C400 + 0x020031ec), (0x020031ec - 0xFE3C00), WUPS_FP_TARGET_PROCESS_GAME_AND_MENU);
WUPS_MUST_REPLACE_PHYSICAL_FOR_PROCESS(EntryHook, (0x3001C400 + 0x020080a8), (0x020080a8 - 0xFE3C00), WUPS_FP_TARGET_PROCESS_GAME_AND_MENU);
WUPS_MUST_REPLACE_PHYSICAL_FOR_PROCESS(__OSGetAppFlags, (0x3001C400 + 0x02003308), (0x02003308 - 0xFE3C00), WUPS_FP_TARGET_PROCESS_GAME_AND_MENU);