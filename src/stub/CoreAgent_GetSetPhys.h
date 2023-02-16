#pragma once

#define CoreAgent_GetPhyReg32 ((uint32_t(*)(uint32_t))(0x101C400 + 0x02067b20 - 0x02000000))
#define CoreAgent_SetPhyReg32 ((uint32_t(*)(uint32_t, uint32_t))(0x101C400 + 0x02067b98 - 0x02000000))
#define CoreAgent_PeekWord32  ((uint32_t(*)(void *))(0x101C400 + 0x02067cc0 - 0x02000000))