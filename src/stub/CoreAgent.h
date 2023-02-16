#pragma once

#define __CoreAgent_SaveState     ((void (*)(OSExceptionType exceptionType, OSContext * interruptedContext, OSContext * cbContext))(0x101C400 + 0x02068408 - 0x02000000))
#define CoreAgent_ResumeCore      ((void (*)(GDBCoreInfo *))(0x101C400 + 0x02067f60 - 0x02000000))
#define CoreAgent_SetMasterReq    ((void (*)(GDBCoreInfo *, GDBStub_Req))(0x101C400 + 0x02067ac8 - 0x02000000))
#define CoreAgent_MasterAgentLoop ((void (*)(GDBCoreInfo *))(0x101C400 + 0x020683e0 - 0x02000000))
#define CoreAgent_SetCoreState    ((void (*)(GDBCoreInfo *, GDBStubState))(0x101C400 + 0x020679c8 - 0x02000000))
#define CoreAgent_SetCmdDone      ((void (*)(GDBCoreInfo *))(0x101C400 + 0x02067ab0 - 0x02000000))