#include "CoreAgent.h"
#include "CoreAgent_GetSetPhys.h"
#include "EnumToStringUtil.h"
#include "GDBDefines.h"
#include "GDBThreadDefines.h"
#include "GDBUtils.h"
#include "imports.h"
#include <coreinit/core.h>
#include <coreinit/debug.h>
#include <wups/function_patching.h>

static const char exception_print_formats[18][45] = {
        "Exception type %d occurred!\n",                 // 0
        "GPR00 %08X GPR08 %08X GPR16 %08X GPR24 %08X\n", // 1
        "GPR01 %08X GPR09 %08X GPR17 %08X GPR25 %08X\n", // 2
        "GPR02 %08X GPR10 %08X GPR18 %08X GPR26 %08X\n", // 3
        "GPR03 %08X GPR11 %08X GPR19 %08X GPR27 %08X\n", // 4
        "GPR04 %08X GPR12 %08X GPR20 %08X GPR28 %08X\n", // 5
        "GPR05 %08X GPR13 %08X GPR21 %08X GPR29 %08X\n", // 6
        "GPR06 %08X GPR14 %08X GPR22 %08X GPR30 %08X\n", // 7
        "GPR07 %08X GPR15 %08X GPR23 %08X GPR31 %08X\n", // 8
        "LR    %08X SRR0  %08x SRR1  %08x\n",            // 9
        "DAR   %08X DSISR %08X\n",                       // 10
        "\nSTACK DUMP:",                                 // 11
        " --> ",                                         // 12
        " -->\n",                                        // 13
        "\n",                                            // 14
        "%p",                                            // 15
        "\nCODE DUMP:\n",                                // 16
        "%p:  %08X %08X %08X %08X\n",                    // 17
};

DECL_FUNCTION(void, __CoreAgent_SaveState, OSExceptionType exceptionType, OSContext *interruptedContext, OSContext *cbContext) {
    auto *currentCoreInfo = &gCoreData[OSGetCoreId()];
    currentCoreInfo->exception_level++;
    if (currentCoreInfo->exception_level != 1) {
        gIsDebuggerPresent = 0;

        CoreAgent_SetCoreState(currentCoreInfo, GDBSTUB_STATE_STOPPING);

        const char *FPAsString = "OFF";
        const char *PRAsString = "";
        if ((interruptedContext->srr1 & MSR_BIT_FP) != 0) {
            FPAsString = "ON";
        }
        if ((interruptedContext->srr1 & MSR_BIT_PR) != 0) {
            PRAsString = " USER";
        }
        auto *exceptionAsStr = GDBStub_OSExceptionToString(exceptionType);
        OSReport("PANIC       %s context:$%08x SRR0:$%08x SRR1:$%08x FP:%s%s\n",
                 exceptionAsStr, interruptedContext, interruptedContext->srr0,
                 interruptedContext->srr1, FPAsString, PRAsString);

        OSReport(
                "DOUBLE FAULT: __CoreAgent_SaveState(%d %08x) core->exception_level (%d) !=  1\n",
                exceptionType,
                interruptedContext,
                currentCoreInfo->exception_level);

        OSReport(exception_print_formats[1], interruptedContext->gpr[0], interruptedContext->gpr[8], interruptedContext->gpr[16], interruptedContext->gpr[24]);
        OSReport(exception_print_formats[2], interruptedContext->gpr[1], interruptedContext->gpr[9], interruptedContext->gpr[17], interruptedContext->gpr[25]);
        OSReport(exception_print_formats[3], interruptedContext->gpr[2], interruptedContext->gpr[10], interruptedContext->gpr[18], interruptedContext->gpr[26]);
        OSReport(exception_print_formats[4], interruptedContext->gpr[3], interruptedContext->gpr[11], interruptedContext->gpr[19], interruptedContext->gpr[27]);
        OSReport(exception_print_formats[5], interruptedContext->gpr[4], interruptedContext->gpr[12], interruptedContext->gpr[20], interruptedContext->gpr[28]);
        OSReport(exception_print_formats[6], interruptedContext->gpr[5], interruptedContext->gpr[13], interruptedContext->gpr[21], interruptedContext->gpr[29]);
        OSReport(exception_print_formats[7], interruptedContext->gpr[6], interruptedContext->gpr[14], interruptedContext->gpr[22], interruptedContext->gpr[30]);
        OSReport(exception_print_formats[8], interruptedContext->gpr[7], interruptedContext->gpr[15], interruptedContext->gpr[23], interruptedContext->gpr[31]);
        OSReport(exception_print_formats[9], interruptedContext->lr, interruptedContext->srr0, interruptedContext->srr1);
        if (exceptionType == OS_EXCEPTION_TYPE_DSI) {
            OSReport(exception_print_formats[10], interruptedContext->dsisr, interruptedContext->dar); // this freezes
        }

        do {
            CoreAgent_SetCoreState(currentCoreInfo, GDBSTUB_STATE_HUNG);
        } while (true);
    }
    CoreAgent_SetCoreState(currentCoreInfo, GDBSTUB_STATE_STOPPING);
    currentCoreInfo->stoppedContext = interruptedContext;
    GDBStub_MasterCore();

    currentCoreInfo->stoppedThread = gCurrentThreadOnCore[currentCoreInfo->core];
    if (currentCoreInfo->stoppedThread == (OSThread *) nullptr) {
        auto curCore = currentCoreInfo->core;
        if (curCore == 0) {
            currentCoreInfo->stoppedThread = IDLE_THREAD_CORE_0;
        } else if (curCore == 1) {
            currentCoreInfo->stoppedThread = IDLE_THREAD_CORE_1;
        } else if (curCore == 2) {
            currentCoreInfo->stoppedThread = IDLE_THREAD_CORE_2;
        }
    } else {
        if (currentCoreInfo->stoppedThread != (OSThread *) interruptedContext) {
            // Check if the interruptedContext is on the stack of the stopped thread.
            if (((uint32_t) interruptedContext >= (uint32_t) currentCoreInfo->stoppedThread->stackStart) &&
                ((uint32_t) interruptedContext < (uint32_t) currentCoreInfo->stoppedThread->stackEnd)) {
            }
        }
    }
    currentCoreInfo->currentContext = currentCoreInfo->stoppedContext;
    currentCoreInfo->currentThread  = currentCoreInfo->stoppedThread;

    FlushFPUContext();
    currentCoreInfo->signal      = GDBSIGNAL_SIGTRAP;
    currentCoreInfo->OSException = (char) exceptionType;
    if (currentCoreInfo->dabr != 0) {
        CoreAgent_SetPhyReg32(GDB_REGISTER_DABR, 0);
    }
    if (currentCoreInfo->iabr != 0) {
        CoreAgent_SetPhyReg32(GDB_REGISTER_IABR, 0);
    }
    bool skipSetCoreData = false;
    switch (exceptionType) {
        case OS_EXCEPTION_TYPE_DSI: {
            auto DSISR = CoreAgent_GetPhyReg32(GDB_REGISTER_DSISR);
            if ((currentCoreInfo->dabr != 0) && (DSISR & DSISR_FLAG_DABRMatch) == DSISR_FLAG_DABRMatch) {
                currentCoreInfo->signal = GDBSIGNAL_SIGTRAP;
                currentCoreInfo->dar    = CoreAgent_GetPhyReg32(GDB_REGISTER_DAR);
            } else {
                currentCoreInfo->signal = GDBSIGNAL_SIGSEGV;
            }
            break;
        }
        case OS_EXCEPTION_TYPE_ALIGNMENT: {
            currentCoreInfo->signal = GDBSIGNAL_SIGBUS;
            break;
        }
        case OS_EXCEPTION_TYPE_SYSTEM_RESET:
        case OS_EXCEPTION_TYPE_MACHINE_CHECK:
        case OS_EXCEPTION_TYPE_DECREMENTER: {
            currentCoreInfo->signal = GDBSIGNAL_SIGINT;
            break;
        }
        case OS_EXCEPTION_TYPE_TRACE:
        case OS_EXCEPTION_TYPE_BREAKPOINT: {
            currentCoreInfo->signal = GDBSIGNAL_SIGTRAP;
            break;
        }
        case OS_EXCEPTION_TYPE_ICI: {
            currentCoreInfo->signal = GDBSIGNAL_RESUME;
            break;
        }
        case OS_EXCEPTION_TYPE_PROGRAM: {
            auto curPC = CoreAgent_PeekWord32((void *) interruptedContext->srr0);
            if (curPC == TRAP_INSTRUCTION) {
                currentCoreInfo->signal = GDBSIGNAL_BREAKPT;
            } else if (curPC == DBGSTR_INSTRUCTION) {
                currentCoreInfo->signal = GDBSIGNAL_DBGSTR;
            } else if (curPC == DBGMSG_INSTRUCTION) {
                currentCoreInfo->signal = GDBSIGNAL_DBGMSG;
            } else if (curPC == DBGCTL_INSTRUCTION) {
                currentCoreInfo->signal = GDBSIGNAL_DBGCTL;
            } else {
                currentCoreInfo->signal = GDBSIGNAL_SIGILL;
            }
            break;
        }
        case OS_EXCEPTION_TYPE_ISI:
        case OS_EXCEPTION_TYPE_EXTERNAL_INTERRUPT:
        case OS_EXCEPTION_TYPE_FLOATING_POINT:
        case OS_EXCEPTION_TYPE_SYSTEM_CALL:
        case OS_EXCEPTION_TYPE_PERFORMANCE_MONITOR:
        case OS_EXCEPTION_TYPE_SYSTEM_INTERRUPT:
        default:
            currentCoreInfo->signal = GDBSIGNAL_SIG0;
            skipSetCoreData         = true;
            break;
    }

    if (!skipSetCoreData) {
        currentCoreInfo->cachedMSR              = interruptedContext->srr1 & 0x87c0ffff;
        currentCoreInfo->cachedHID2             = 0;
        currentCoreInfo->cachedPC               = (currentCoreInfo->currentContext)->srr0;
        (currentCoreInfo->currentContext)->srr1 = (currentCoreInfo->currentContext)->srr1 & ~MSR_BIT_SE;
        CoreAgent_SetMasterReq(currentCoreInfo, GDBSTUB_REQ_STOPPED);
        CoreAgent_SetCoreState(currentCoreInfo, GDBSTUB_STATE_STOPPED);
    }

    if (currentCoreInfo->cmd.cmd == GDBSTUB_CMD_SINGLESTEP) {
        (currentCoreInfo->currentContext)->srr1 = (currentCoreInfo->currentContext)->srr1 | (uint32_t) currentCoreInfo->stoppedContextExternalIinterruptBit;
        CoreAgent_SetCmdDone(currentCoreInfo);
    }
    CoreAgent_MasterAgentLoop(currentCoreInfo);
    do {
        CoreAgent_SetCoreState(currentCoreInfo, GDBSTUB_STATE_HUNG);
    } while (true);
}

DECL_FUNCTION(void, CoreAgent_ResumeCore, GDBCoreInfo *curCoreData) {
    auto stoppedContext = curCoreData->stoppedContext;

    if (curCoreData->req.ack != curCoreData->reqAckValue) {
        return;
    }

    CoreAgent_SetCoreState(curCoreData, GDBSTUB_STATE_PROCESSING);

    uint32_t curPCInstruction = CoreAgent_PeekWord32((void *) stoppedContext->srr0);
    if (curPCInstruction == TRAP_INSTRUCTION ||
        curPCInstruction == DBGSTR_INSTRUCTION ||
        curPCInstruction == DBGMSG_INSTRUCTION ||
        curPCInstruction == DBGCTL_INSTRUCTION) {
        stoppedContext->srr0 = stoppedContext->srr0 + 4;
    }

    if (curCoreData->cmd.cmd == GDBSTUB_CMD_CONTINUE) {
        CoreAgent_SetCoreState(curCoreData, GDBSTUB_STATE_RUNNING);
        curCoreData->stoppedContextExternalIinterruptBit = 0;
        stoppedContext->srr1                             = stoppedContext->srr1 & ~MSR_BIT_SE;
    } else if (curCoreData->cmd.cmd == GDBSTUB_CMD_SINGLESTEP) {
        CoreAgent_SetCoreState(curCoreData, GDBSTUB_STATE_SINGLESTEP);
        curCoreData->stoppedContextExternalIinterruptBit = ((uint16_t) stoppedContext->srr1) | MSR_BIT_EE;
        stoppedContext->srr1                             = (stoppedContext->srr1 & ~MSR_BIT_EE) | MSR_BIT_SE;
    }

    if (curCoreData->cmd.cmd == GDBSTUB_CMD_CONTINUE) {
        CoreAgent_SetCmdDone(curCoreData);
    }
    curCoreData->exception_level--;
    gTimerKernelCallbackLastRun = in_TBLr();
    if (curCoreData->dabr != 0) {
        CoreAgent_SetPhyReg32(GDB_REGISTER_DABR, curCoreData->dabr);
    }
    if (curCoreData->iabr != 0) {
        CoreAgent_SetPhyReg32(GDB_REGISTER_IABR, curCoreData->iabr);
    }
    if (curCoreData->cmd.cmd == GDBSTUB_CMD_CONTINUE) {
        __OSSetAndLoadContext(stoppedContext);
        return;
    }
    __OSSetAndLoadContextDebugger(stoppedContext);
}

WUPS_MUST_REPLACE_PHYSICAL_FOR_PROCESS(__CoreAgent_SaveState, (0x3201C400 + (0x02068408 - 0x02000000)), (0x101C400 + (0x02068408 - 0x02000000)), WUPS_FP_TARGET_PROCESS_ALL);
WUPS_MUST_REPLACE_PHYSICAL_FOR_PROCESS(CoreAgent_ResumeCore, (0x3201C400 + (0x02067f60 - 0x02000000)), (0x101C400 + (0x02067f60 - 0x02000000)), WUPS_FP_TARGET_PROCESS_ALL);
