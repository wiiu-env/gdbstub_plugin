#pragma once

#define MasterAgent_FlushICBlock ((uint32_t(*)(GDBCoreInfo * curCoreInfo, uint32_t addr))(0x101C400 + 0x02038970 - 0x02000000))