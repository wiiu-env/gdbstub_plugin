#include "CoreAgent_MemorySetGet.h"
#include "utils/kernel.h"
#include <coreinit/cache.h>
#include <coreinit/debug.h>
#include <coreinit/memorymap.h>
#include <cstring>
#include <wups/function_patching.h>

DECL_FUNCTION(uint32_t, CoreAgent_GetCoreMem, void *addr, uint32_t size, void *outRes) {
    if (OSIsAddressValid((uint32_t) addr)) {
        KernelWrite((uint32_t) outRes, addr, size);
    } else {
        memset(outRes, 0, size);
        OSReport("CoreAgent_GetCoreMem invalid addr: %08X\n", addr);
    }

    return 0;
}

DECL_FUNCTION(uint32_t, CoreAgent_SetCoreMem, void *addr, uint32_t size, void *srcBuf) {
    KernelWrite((uint32_t) addr, srcBuf, size);
    ICInvalidateRange(addr, size);
    DCFlushRange(addr, size);

    return 0;
}

WUPS_MUST_REPLACE_PHYSICAL_FOR_PROCESS(CoreAgent_GetCoreMem, (0x3201C400 + (0x02067bd4 - 0x02000000)), (0x101C400 + (0x02067bd4 - 0x02000000)), WUPS_FP_TARGET_PROCESS_ALL);
WUPS_MUST_REPLACE_PHYSICAL_FOR_PROCESS(CoreAgent_SetCoreMem, (0x3201C400 + (0x02067cf8 - 0x02000000)), (0x101C400 + (0x02067cf8 - 0x02000000)), WUPS_FP_TARGET_PROCESS_ALL);
