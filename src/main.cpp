#include "debugger.h"
#include "exceptions.h"
#include "logger.h"
#include <coreinit/cache.h>
#include <coreinit/debug.h>
#include <coreinit/dynload.h>
#include <coreinit/memorymap.h>
#include <kernel/kernel.h>
#include <wups.h>

OSThread **pThreadList;

WUPS_PLUGIN_NAME("Debugger");
WUPS_PLUGIN_DESCRIPTION("Wii U Debugger");
WUPS_PLUGIN_VERSION("0.1");
WUPS_PLUGIN_AUTHOR("Kinnay, Maschell");
WUPS_PLUGIN_LICENSE("GPL");

WUPS_USE_WUT_DEVOPTAB();

/* https://github.com/QuarkTheAwesome/CafeBinPatch/blob/main/src/runtime-patcher.cpp#L58 */
bool PatchInstruction(void *instr, uint32_t original, uint32_t replacement) {
    uint32_t current = *(uint32_t *) instr;
    if (current != original) return current == replacement;

    KernelCopyData(OSEffectiveToPhysical((uint32_t) instr), OSEffectiveToPhysical((uint32_t) &replacement), sizeof(replacement));
    //Only works on AROMA! WUPS 0.1's KernelCopyData is uncached, needs DCInvalidate here instead
    DCFlushRange(instr, 4);
    ICInvalidateRange(instr, 4);

    current = *(uint32_t *) instr;

    return true;
}

/* https://github.com/QuarkTheAwesome/CafeBinPatch/blob/main/src/runtime-patcher.cpp#L74 */
bool PatchDynLoadFunctions() {
    uint32_t *patch1 = ((uint32_t *) &OSDynLoad_GetNumberOfRPLs) + 6;
    uint32_t *patch2 = ((uint32_t *) &OSDynLoad_GetRPLInfo) + 22;

    if (!PatchInstruction(patch1, 0x41820038 /* beq +38 */, 0x60000000 /*nop*/)) {
        return false;
    }
    if (!PatchInstruction(patch2, 0x41820100 /* beq +100 */, 0x60000000 /*nop*/)) {
        return false;
    }

    return true;
}


INITIALIZE_PLUGIN() {
    PatchDynLoadFunctions();
}

ON_APPLICATION_START() {
    initLogging();
    DEBUG_FUNCTION_LINE("Hello from Debugger plugin");
    pThreadList = (OSThread **) 0x100567F8; // 100567f8
    debugger    = new Debugger();
    DCFlushRange(&debugger, 4);
    DCFlushRange(debugger, sizeof(Debugger));
    DEBUG_FUNCTION_LINE("Created Debugger");
    debugger->start();
    DEBUG_FUNCTION_LINE("Started Debugger thread");
}

ON_APPLICATION_REQUESTS_EXIT() {
    DEBUG_FUNCTION_LINE("Deleting Debugger thread");
    delete debugger;
    DEBUG_FUNCTION_LINE("Deleted Debugger thread");
    deinitLogging();
}
ON_APPLICATION_ENDS() {
    initDebugState = false;
    DCFlushRange(&initDebugState, sizeof(initDebugState));
}
