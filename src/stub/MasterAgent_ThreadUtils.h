#pragma once

#define MasterAgent_GetIndexedThreadID ((OSThread * (*) (int32_t))(0x101C400 + 0x0203a3e0 - 0x02000000))
#define MasterAgent_GetThreadID        ((uint32_t(*)(OSThread *))(0x101C400 + 0x0203a290 - 0x02000000))
#define MasterAgent_ThreadExtraInfo    ((const char *(*) (OSThread *) )(0x101C400 + 0x0203a548 - 0x02000000))
