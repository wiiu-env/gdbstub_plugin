#include <coreinit/cache.h>
#include <coreinit/memorymap.h>
#include <cstdint>
#include <kernel/kernel.h>

void KernelWrite(uint32_t addr, const void *data, uint32_t length) {
    uint32_t dst = OSEffectiveToPhysical(addr);
    uint32_t src = OSEffectiveToPhysical((uint32_t) data);
    KernelCopyData(dst, src, length);
    DCFlushRange((void *) addr, length);
    ICInvalidateRange((void *) addr, length);
}

void KernelWriteU32(uint32_t addr, uint32_t value) {
    uint32_t dst = OSEffectiveToPhysical(addr);
    uint32_t src = OSEffectiveToPhysical((uint32_t) &value);
    KernelCopyData(dst, src, 4);
    DCFlushRange((void *) addr, 4);
    ICInvalidateRange((void *) addr, 4);
}
