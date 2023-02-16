#pragma once
#include <cstdint>

#define GDBStub_MasterCore            ((uint32_t(*)(void))(0x101C400 + 0x02036bc8 - 0x02000000))
#define FlushFPUContext               ((void (*)(void))(0x101C400 + 0x0203ee5c - 0x02000000))
#define __OSSetAndLoadContextDebugger ((void (*)(OSContext *))(0x101C400 + 0x02008188 - 0x02000000))

extern "C" uint32_t in_TBLr();