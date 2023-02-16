#include "MasterAgent_Utils.h"
#include "GDBDefines.h"
#include "GDBUtils.h"
#include "imports.h"
#include "utils/kernel.h"
#include <cerrno>
#include <coreinit/debug.h>
#include <coreinit/memorymap.h>
#include <wups/function_patching.h>

DECL_FUNCTION(uint32_t, MasterAgent_PeekWord32, void *addr) {
    uint32_t value = 0xbaadf00d;
    if (OSIsAddressValid((uint32_t) &value)) {
        KernelWrite((uint32_t) &value, addr, 4);
    } else {
        OSReport("MasterAgent_PeekWord32 invalid addr: %08X\n", addr);
    }
    return value;
}

DECL_FUNCTION(uint32_t, MasterAgent_PokeInstr, void *addr, uint32_t newInstruction) {
    if (OSIsAddressValid((uint32_t) addr)) {
        KernelWriteU32((uint32_t) addr, newInstruction);
        for (uint32_t i = 0; i < 3; i++) {
            if (gCoreData[i].state == GDBSTUB_STATE_STOPPED) {
                if (i != GDBStub_MasterCore()) {
                    MasterAgent_FlushICBlock(&gCoreData[i], (uint32_t) addr);
                }
            }
        }
    } else {
        OSReport("MasterAgent_PokeInstr invalid addr: %08X\n", addr);
        return EINVAL;
    }
    return 0;
}

WUPS_MUST_REPLACE_PHYSICAL_FOR_PROCESS(MasterAgent_PeekWord32, (0x3201C400 + (0x0203881c - 0x02000000)), (0x101C400 + (0x0203881c - 0x02000000)), WUPS_FP_TARGET_PROCESS_ALL);
WUPS_MUST_REPLACE_PHYSICAL_FOR_PROCESS(MasterAgent_PokeInstr, (0x3201C400 + (0x020389cc - 0x02000000)), (0x101C400 + (0x020389cc - 0x02000000)), WUPS_FP_TARGET_PROCESS_ALL);
