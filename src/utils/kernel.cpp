#include <coreinit/cache.h>
#include <coreinit/debug.h>
#include <coreinit/memorymap.h>
#include <kernel/kernel.h>

void KernelWrite(uint32_t addr, const void *data, uint32_t length) {
    uint32_t dst = OSEffectiveToPhysical(addr);
    uint32_t src = OSEffectiveToPhysical((uint32_t) data);
    if (dst == 0 || src == 0) {
        OSReport("Attempted to read/write from 0 (phys). dst %08X src %08X\n", dst, src);
        return;
    }
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


#define KiInitRendezvous      ((void (*)(void *)) 0xfff097c4)
#define KiSendICIToOtherCores ((void (*)(uint32_t, uint32_t, uint32_t, uint32_t)) 0xfff052c8)
#define KiWaitRendezvous      ((void (*)(void *, uint32_t)) 0xfff097e0)

void KeSetDABR(uint32_t addr, int32_t allCores, int32_t matchReads, int32_t matchWrite) {
    auto maskedAddr = (addr & 0xfffffff8) | 4;
    if (matchReads != 0) {
        maskedAddr = maskedAddr | 5;
    }
    if (matchWrite != 0) {
        maskedAddr = maskedAddr | 2;
    }

    asm volatile(
            " mtspr 1013, %0 \n"
            :
            : "r"(maskedAddr)
            :);

    if (allCores != 0) {
        register int r13 asm("r13");
        KiInitRendezvous(reinterpret_cast<void *>(r13 + -0x72a0));
        KiSendICIToOtherCores(5, maskedAddr, 0, 1);
        KiWaitRendezvous(reinterpret_cast<void *>(r13 + -0x72a0), 7);
    }
}

void KeSetIABR(uint32_t addr, int32_t allCores) {
    auto maskedAddr = (addr & 0xfffffffc) | 3;

    asm volatile(
            " mtspr 1010, %0 \n"
            :
            : "r"(maskedAddr)
            :);

    if (allCores != 0) {
        register int r13 asm("r13");
        KiInitRendezvous(reinterpret_cast<void *>(r13 + -0x7290));
        KiSendICIToOtherCores(10, maskedAddr, 0, 1);
        KiWaitRendezvous(reinterpret_cast<void *>(r13 + -0x7290), 7);
    }
}

void EnableIABRandDABRSupport() {
    KernelPatchSyscall(0x4B, reinterpret_cast<uint32_t>(&KeSetDABR));
    KernelPatchSyscall(0x4C, reinterpret_cast<uint32_t>(&KeSetIABR));
}

void DisableIABRandDABRSupport() {
    // Restore original syscalls
    KernelPatchSyscall(0x4B, 0xfff036a0);
    KernelPatchSyscall(0x4C, 0xfff0370c);
}