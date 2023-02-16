#include "stub/GDBDefines.h"
#include <cstdint>

auto *gGDBGHSShowDrvrThreadsPtr      = (uint32_t *) 0x1004f9bc;
auto *gAppflagsPtr                   = (uint32_t *) 0x10013c2c;
auto *gIsDebuggerPresentPtr          = (uint32_t *) 0x1003aff8;
auto *DAT_100d1378Ptr                = (uint32_t *) 0x100d1378;
auto *g100msInTicksPtr               = (uint32_t *) 0x100523d8;
auto *gTimerKernelCallbackLastRunPtr = (uint32_t *) 0x100523d0;
auto *DAT_100d1374Ptr                = (uint32_t *) 0x100d1374;
auto *gCoreCountPtr                  = (uint32_t *) 0x1004f9b8;
auto *gGDBStub_main_statePtr         = (GDBMainState *) 0x1004f9b4;
auto *gIsDebuggerInitializedOnCore   = (uint32_t *) 0x1004f974;
auto *gCoreData                      = (GDBCoreInfo *) 0x1004fb38;
auto *gCurrentThreadOnCore           = (OSThread **) 0x10057c00;
GDBCoreInfo **gGDBCurCoreInfoPtr     = (GDBCoreInfo **) 0x1004f9e4;
GDBCoreInfo **gGDBStoppedCoreInfoPtr = (GDBCoreInfo **) 0x1004f9e8;