#pragma once
#include <cstdint>

#define CoreAgent_GetCoreMem ((uint32_t(*)(void *addr, uint32_t size, void *outRes))(0x101C400 + 0x02067bd4 - 0x02000000))
#define CoreAgent_SetCoreMem ((uint32_t(*)(void *addr, uint32_t size, void *srcBuf))(0x101C400 + 0x02067cf8 - 0x02000000))