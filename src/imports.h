#pragma once
#include "stub/GDBDefines.h"
#include <coreinit/thread.h>
#include <cstdint>

extern uint32_t *gGDBGHSShowDrvrThreadsPtr;
extern uint32_t *gAppflagsPtr;
extern uint32_t *gIsDebuggerPresentPtr;
extern uint32_t *DAT_100d1378Ptr;
extern uint32_t *DAT_100d1374Ptr;
extern uint32_t *gCoreCountPtr;
extern GDBMainState *gGDBStub_main_statePtr;
extern uint32_t *g100msInTicksPtr;
extern uint32_t *gTimerKernelCallbackLastRunPtr;
extern GDBCoreInfo **gGDBCurCoreInfoPtr;
extern GDBCoreInfo **gGDBStoppedCoreInfoPtr;
extern OSThread **gCurrentThreadOnCore;

#define gGDBGHSShowDrvrThreads      (*gGDBGHSShowDrvrThreadsPtr)
#define gAppflags                   (*gAppflagsPtr)
#define gIsDebuggerPresent          (*gIsDebuggerPresentPtr)
#define DAT_100d1378                (*DAT_100d1378Ptr)
#define DAT_100d1374                (*DAT_100d1374Ptr)
#define gGDBStub_main_state         (*gGDBStub_main_statePtr)
#define gCoreCount                  (*gCoreCountPtr)
#define g100msInTicks               (*g100msInTicksPtr)
#define gTimerKernelCallbackLastRun (*gTimerKernelCallbackLastRunPtr)

#define gGDBCurCoreInfo             (*gGDBCurCoreInfoPtr)
#define gGDBStoppedCoreInfo         (*gGDBStoppedCoreInfoPtr)

extern uint32_t *gIsDebuggerInitializedOnCore;
extern GDBCoreInfo *gCoreData;

extern "C" uint32_t __OSGetAppFlags();

#define OSDisableUserHeartbeat ((void (*)(void))(0x101C400 + 0x155b0))
