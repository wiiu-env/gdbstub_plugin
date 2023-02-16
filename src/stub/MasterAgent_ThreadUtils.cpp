#include "GDBThreadDefines.h"
#include "GDBUtils.h"
#include "imports.h"
#include <coreinit/thread.h>
#include <cstdio>
#include <wups/function_patching.h>
static char sSomeStringBuffer[100] = {};

DECL_FUNCTION(const char *, MasterAgent_ThreadExtraInfo, OSThread *thread) {
    const char *result;
    if (thread == IDLE_THREAD_CORE_0 || thread == IDLE_THREAD_CORE_1 || thread == IDLE_THREAD_CORE_2) {
        if (thread == IDLE_THREAD_CORE_0) {
            result = "core 0 *running priority = (32:32) name = Idle Core 0";
        } else if (thread == IDLE_THREAD_CORE_1) {
            result = "core 1 *running priority = (32:32) name = Idle Core 1";
        } else {
            result = "core 2 *running priority = (32:32) name = Idle Core 2";
        }
    } else {
        GDBStub_MasterCore();
        uint32_t coreOfThread;

        if ((thread->attr & OS_THREAD_ATTRIB_AFFINITY_ANY) == OS_THREAD_ATTRIB_AFFINITY_CPU0) {
            coreOfThread = 0;
        } else if ((thread->attr & OS_THREAD_ATTRIB_AFFINITY_ANY) == OS_THREAD_ATTRIB_AFFINITY_CPU1) {
            coreOfThread = 1;
        } else if ((thread->attr & OS_THREAD_ATTRIB_AFFINITY_ANY) == OS_THREAD_ATTRIB_AFFINITY_CPU2) {
            coreOfThread = 2;
        } else {
            coreOfThread = thread->context.upir;
        }

        uint32_t threadState;
        threadState = thread->state;

        auto *threadStateAsStr = "";
        if (gCoreData[coreOfThread].state == GDBSTUB_STATE_HUNG) {
            threadStateAsStr = "HUNG ";
        }
        if ((gGDBCurCoreInfo != nullptr) &&
            (thread == gGDBStoppedCoreInfo->stoppedThread)) {
            threadStateAsStr = "*";
        }

        auto threadReadyStr = "";
        if ((threadState & OS_THREAD_STATE_READY) != 0) {
            threadReadyStr = "ready ";
        }
        auto threadRunningStr = "";
        if ((threadState & OS_THREAD_STATE_RUNNING) != 0) {
            threadRunningStr = "running ";
        }
        auto threadWaitingStr = "";
        if ((threadState & OS_THREAD_STATE_WAITING) != 0) {
            threadWaitingStr = "waiting ";
        }
        auto threadMoribundStr = "";
        if ((threadState & OS_THREAD_STATE_MORIBUND) != 0) {
            threadMoribundStr = "moribund ";
        }
        auto threadDetachStr = "";
        if ((thread->attr & OS_THREAD_ATTRIB_DETACHED) != 0) {
            threadDetachStr = "detach ";
        }
        char threadNameOnStack[0x18];
        auto *threadName = thread->name;
        if (threadName == nullptr) {
            snprintf(threadNameOnStack, 0x18, "Thread 0x%8.8x", (uint32_t) thread);
            threadName = threadNameOnStack;
        }
        snprintf(sSomeStringBuffer, 100, "core %d %s%s%s%s%s%spriority = (%d:%d) name = %s",
                 coreOfThread,
                 threadStateAsStr,
                 threadRunningStr,
                 threadReadyStr,
                 threadWaitingStr,
                 threadMoribundStr,
                 threadDetachStr,
                 thread->priority,
                 thread->basePriority,
                 threadName);
        result = sSomeStringBuffer;
    }
    return result;
}

WUPS_MUST_REPLACE_PHYSICAL_FOR_PROCESS(MasterAgent_ThreadExtraInfo, (0x3201C400 + (0x0203a548 - 0x02000000)), (0x101C400 + (0x0203a548 - 0x02000000)), WUPS_FP_TARGET_PROCESS_ALL);
