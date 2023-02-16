#pragma once

#define MasterAgent_GetRegister64 ((uint64_t(*)(GDBCoreInfo * curInfo, OSContext * context, uint32_t regId))(0x101C400 + 0x02039e60 - 0x02000000))
#define MasterAgent_GetRegister32 ((uint32_t(*)(GDBCoreInfo * curInfo, OSContext * context, uint32_t regId))(0x101C400 + 0x02038cf0 - 0x02000000))
#define MasterAgent_SetRegister32 ((uint32_t(*)(GDBCoreInfo * coreInfo, OSContext * context, uint32_t regId, uint32_t value))(0x101C400 + 0x02039ed0 - 0x02000000))
