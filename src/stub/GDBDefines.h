#pragma once
#include <coreinit/context.h>
#include <coreinit/kernel.h>
#include <coreinit/rendezvous.h>
#include <coreinit/thread.h>
#include <cstdint>
#include <wut.h>

typedef enum GDBStub_CMD : uint8_t {
    GDBSTUB_CMD_IDLE         = 0,
    GDBSTUB_CMD_STOP         = 1,
    GDBSTUB_CMD_CONTINUE     = 2,
    GDBSTUB_CMD_SINGLESTEP   = 3,
    GDBSTUB_CMD_GETSYSREG    = 4,
    GDBSTUB_CMD_PUTSYSREG    = 5,
    GDBSTUB_CMD_ICINVALIDBLK = 6,
    GDBSTUB_CMD_DCINVALRNG   = 7,
    GDBSTUB_CMD_GETCOREMEM   = 8,
    GDBSTUB_CMD_PUTCOREMEM   = 9
} GDBStub_CMD;

typedef enum GDBMainState {
    GDBMAINSTATE_LOOP      = 0,
    GDBMAINSTATE_STEP      = 1,
    GDBMAINSTATE_RUN       = 2,
    GDBMAINSTATE_VCONT     = 3,
    GDBMAINSTATE_MAX_VALUE = 4
} GDBMainState;

typedef enum GDBStub_GDBFlags : uint8_t {
    GDBFLAGS_ZERO    = 0,
    GDBFLAGS_GDBSTOP = 1,
    GDBFLAGS_GDBSTEP = 2,
    GDBFLAGS_INVAL   = 3,
    GDBFLAGS_GDBCONT = 4
} GDBStub_GDBFlags;

typedef enum GDB_SIGNAL : uint16_t {
    GDBSIGNAL_SIG0    = 0,
    GDBSIGNAL_SIGINT  = 2,
    GDBSIGNAL_SIGQUIT = 3,
    GDBSIGNAL_SIGILL  = 4,
    GDBSIGNAL_SIGTRAP = 5,
    GDBSIGNAL_SIGKILL = 9,
    GDBSIGNAL_SIGBUS  = 10,
    GDBSIGNAL_SIGSEGV = 11,
    GDBSIGNAL_SIGTERM = 15,
    GDBSIGNAL_SIGSTOP = 17,
    GDBSIGNAL_RESUME  = 65437,
    GDBSIGNAL_DBGCTL  = 65445,
    GDBSIGNAL_DBGMSG  = 65446,
    GDBSIGNAL_BREAKPT = 65534,
    GDBSIGNAL_DBGSTR  = 65535
} GDB_SIGNAL;

typedef enum GDBStub_ACCESS {
    GDBSTUB_ACCESS_NONE  = 0,
    GDBSTUB_ACCESS_READ  = 1,
    GDBSTUB_ACCESS_WRITE = 2
} GDBStub_ACCESS;

typedef enum GDBStubState {
    GDBSTUB_STATE_INACTIVE   = 0,
    GDBSTUB_STATE_STOPPED    = 1,
    GDBSTUB_STATE_RUNNING    = 2,
    GDBSTUB_STATE_STOPPING   = 3,
    GDBSTUB_STATE_SINGLESTEP = 4,
    GDBSTUB_STATE_PROCESSING = 5,
    GDBSTUB_STATE_HUNG       = 6
} GDBStubState;

typedef enum GDBStub_Req : uint8_t {
    GDBSTUB_REQ_STOPPED  = 0,
    GDBSTUB_REQ_FINISHED = 1
} GDBStub_Req;

typedef struct WUT_PACKED CMDStruct {
    GDBStub_CMD cmd;
    struct WUT_PACKED {
        unsigned int ack : 24;
    };
} CMDStruct;
WUT_CHECK_OFFSET(CMDStruct, 0, cmd);
WUT_CHECK_SIZE(CMDStruct, 0x4);

typedef struct WUT_PACKED REQStruct {
    GDBStub_Req req;
    struct WUT_PACKED {
        unsigned int ack : 24;
    };
} REQStruct;
WUT_CHECK_SIZE(REQStruct, 0x4);

typedef struct WUT_PACKED GDBCoreInfo {
    uint8_t core;
    GDBStub_GDBFlags flag;
    uint16_t stoppedContextExternalIinterruptBit;
    GDB_SIGNAL nextSignal__;
    GDB_SIGNAL signal;
    GDBStub_ACCESS accessType;
    uint32_t accessLevel;
    char OSException;
    WUT_UNKNOWN_BYTES(3);
    int32_t exception_level;
    uint32_t numPrintedMessages;
    uint32_t totalMessagesInQueue;
    OSContext *exceptionContext;
    void *exceptionStack;
    GDBStubState state;
    uint32_t lastStateUpdateTime;
    REQStruct req;
    uint32_t reqAckValue;
    uint32_t req_arg;
    uint32_t lastReqSetTime;
    uint32_t reqDoneTime;
    CMDStruct cmd;
    uint32_t cmdAckValue;
    uint32_t cmdArg1;
    uint32_t cmdArg2;
    uint32_t cmdResult;
    uint32_t cmdDoneTime;
    uint32_t cmdSetAtTime;
    OSThread *stoppedThread;
    OSContext *stoppedContext;
    OSThread *currentThread;
    OSContext *currentContext;
    uint32_t virtual_ssr0;
    uint32_t virtual_ssr1;
    uint32_t cachedHID2;
    uint32_t cachedMSR;
    uint32_t cachedPC;
    uint32_t dabr;
    uint32_t iabr;
    uint32_t dar;
} GDBCoreInfo;
WUT_CHECK_OFFSET(GDBCoreInfo, 0, core);
WUT_CHECK_OFFSET(GDBCoreInfo, 1, flag);
WUT_CHECK_OFFSET(GDBCoreInfo, 2, stoppedContextExternalIinterruptBit);
WUT_CHECK_OFFSET(GDBCoreInfo, 4, nextSignal__);
WUT_CHECK_OFFSET(GDBCoreInfo, 6, signal);
WUT_CHECK_OFFSET(GDBCoreInfo, 8, accessType);
WUT_CHECK_OFFSET(GDBCoreInfo, 12, accessLevel);
WUT_CHECK_OFFSET(GDBCoreInfo, 20, exception_level);
WUT_CHECK_OFFSET(GDBCoreInfo, 24, numPrintedMessages);
WUT_CHECK_OFFSET(GDBCoreInfo, 28, totalMessagesInQueue);
WUT_CHECK_OFFSET(GDBCoreInfo, 32, exceptionContext);
WUT_CHECK_OFFSET(GDBCoreInfo, 36, exceptionStack);
WUT_CHECK_OFFSET(GDBCoreInfo, 40, state);
WUT_CHECK_OFFSET(GDBCoreInfo, 44, lastStateUpdateTime);
WUT_CHECK_OFFSET(GDBCoreInfo, 48, req);
WUT_CHECK_OFFSET(GDBCoreInfo, 52, reqAckValue);
WUT_CHECK_OFFSET(GDBCoreInfo, 56, req_arg);
WUT_CHECK_OFFSET(GDBCoreInfo, 60, lastReqSetTime);
WUT_CHECK_OFFSET(GDBCoreInfo, 64, reqDoneTime);
WUT_CHECK_OFFSET(GDBCoreInfo, 68, cmd);
WUT_CHECK_OFFSET(GDBCoreInfo, 72, cmdAckValue);
WUT_CHECK_OFFSET(GDBCoreInfo, 76, cmdArg1);
WUT_CHECK_OFFSET(GDBCoreInfo, 80, cmdArg2);
WUT_CHECK_OFFSET(GDBCoreInfo, 84, cmdResult);
WUT_CHECK_OFFSET(GDBCoreInfo, 88, cmdDoneTime);
WUT_CHECK_OFFSET(GDBCoreInfo, 92, cmdSetAtTime);
WUT_CHECK_OFFSET(GDBCoreInfo, 96, stoppedThread);
WUT_CHECK_OFFSET(GDBCoreInfo, 100, stoppedContext);
WUT_CHECK_OFFSET(GDBCoreInfo, 104, currentThread);
WUT_CHECK_OFFSET(GDBCoreInfo, 108, currentContext);
WUT_CHECK_OFFSET(GDBCoreInfo, 112, virtual_ssr0);
WUT_CHECK_OFFSET(GDBCoreInfo, 116, virtual_ssr1);
WUT_CHECK_OFFSET(GDBCoreInfo, 120, cachedHID2);
WUT_CHECK_OFFSET(GDBCoreInfo, 124, cachedMSR);
WUT_CHECK_OFFSET(GDBCoreInfo, 128, cachedPC);
WUT_CHECK_OFFSET(GDBCoreInfo, 132, dabr);
WUT_CHECK_OFFSET(GDBCoreInfo, 136, iabr);
WUT_CHECK_OFFSET(GDBCoreInfo, 140, dar);
WUT_CHECK_SIZE(GDBCoreInfo, 144);

typedef enum DebugQueueType {
    STRING  = 0,
    SET_CMD = 1,
    ACK_CMD = 2,
    SET_REQ = 3,
    ACK_REQ = 4
} DebugQueueType;

typedef struct GDBDebugMessageString {
    const char *format;
    void *args[9];
} GDBDebugMessageString;
WUT_CHECK_OFFSET(GDBDebugMessageString, 0x0, format);
WUT_CHECK_OFFSET(GDBDebugMessageString, 0x4, args);
WUT_CHECK_SIZE(GDBDebugMessageString, 40);

typedef struct GDBDebugMessageAckCmd {
    GDBStub_CMD cmd;
    uint32_t cmdAck;
    uint32_t cmdResult;
} GDBDebugMessageAckCmd;
WUT_CHECK_OFFSET(GDBDebugMessageAckCmd, 0x0, cmd);
WUT_CHECK_OFFSET(GDBDebugMessageAckCmd, 0x4, cmdAck);
WUT_CHECK_OFFSET(GDBDebugMessageAckCmd, 0x8, cmdResult);
WUT_CHECK_SIZE(GDBDebugMessageAckCmd, 0xC);

typedef struct GDBDebugMessageSetReq {
    GDBStub_Req req;
    uint32_t reqAck;
    uint32_t arg;
} GDBDebugMessageSetReq;
WUT_CHECK_OFFSET(GDBDebugMessageSetReq, 0x0, req);
WUT_CHECK_OFFSET(GDBDebugMessageSetReq, 0x4, reqAck);
WUT_CHECK_OFFSET(GDBDebugMessageSetReq, 0x8, arg);
WUT_CHECK_SIZE(GDBDebugMessageSetReq, 0xC);

typedef struct GDBDebugMessageSetCmd {
    uint32_t core;
    GDBStub_CMD cmd;
    uint32_t cmdCount;
    uint32_t arg1;
    uint32_t arg2;
} GDBDebugMessageSetCmd;
WUT_CHECK_OFFSET(GDBDebugMessageSetCmd, 0x0, core);
WUT_CHECK_OFFSET(GDBDebugMessageSetCmd, 0x4, cmd);
WUT_CHECK_OFFSET(GDBDebugMessageSetCmd, 0x8, cmdCount);
WUT_CHECK_OFFSET(GDBDebugMessageSetCmd, 0xC, arg1);
WUT_CHECK_OFFSET(GDBDebugMessageSetCmd, 0x10, arg2);
WUT_CHECK_SIZE(GDBDebugMessageSetCmd, 0x14);

typedef struct GDBDebugMessageAckReq {
    uint32_t core;
    GDBStub_Req req;
    uint32_t reqCount;
} GDBDebugMessageAckReq;
WUT_CHECK_OFFSET(GDBDebugMessageAckReq, 0x0, core);
WUT_CHECK_OFFSET(GDBDebugMessageAckReq, 0x4, req);
WUT_CHECK_OFFSET(GDBDebugMessageAckReq, 0x8, reqCount);
WUT_CHECK_SIZE(GDBDebugMessageAckReq, 0x0C);

typedef struct debug_queue_msg {
    uint64_t timeInTicks;
    uint32_t debugMask;
    DebugQueueType type;
    union {
        GDBDebugMessageString dbgStr;
        GDBDebugMessageAckCmd dbgAckCmd;
        GDBDebugMessageAckReq dbgAckReq;
        GDBDebugMessageSetCmd dbgSetCmd;
        GDBDebugMessageSetReq dbgSetReq;
        WUT_UNKNOWN_BYTES(0x20);
    };
    WUT_PADDING_BYTES(8);
} debug_queue_msg;
WUT_CHECK_OFFSET(debug_queue_msg, 0x0, timeInTicks);
WUT_CHECK_OFFSET(debug_queue_msg, 0x8, debugMask);
WUT_CHECK_OFFSET(debug_queue_msg, 0xC, type);
WUT_CHECK_OFFSET(debug_queue_msg, 0x10, dbgStr);
WUT_CHECK_OFFSET(debug_queue_msg, 0x10, dbgAckCmd);
WUT_CHECK_OFFSET(debug_queue_msg, 0x10, dbgAckReq);
WUT_CHECK_OFFSET(debug_queue_msg, 0x10, dbgSetCmd);
WUT_CHECK_OFFSET(debug_queue_msg, 0x10, dbgSetReq);
WUT_CHECK_SIZE(debug_queue_msg, 64);

typedef enum GDBWatchState : uint8_t {
    GDBWATCHSTATE_UNKNOWN = 0,
    GDBWATCHSTATE_RWATCH  = 5,
    GDBWATCHSTATE_WATCH   = 6,
    GDBWATCHSTATE_AWATCH  = 7,

} GDBWatchState;

#define MSR_BIT_POW                                 (1 << (31 - 13)) /*0x40000*/
#define MSR_BIT_RESERVED0                           (1 << (31 - 14)) /*0x20000*/
#define MSR_BIT_ILE                                 (1 << (31 - 15)) /*0x10000*/
#define MSR_BIT_EE                                  (1 << (31 - 16)) /*0x8000*/
#define MSR_BIT_PR                                  (1 << (31 - 17)) /*0x4000*/
#define MSR_BIT_FP                                  (1 << (31 - 18)) /*0x2000*/
#define MSR_BIT_ME                                  (1 << (31 - 19)) /*0x1000*/
#define MSR_BIT_FE0                                 (1 << (31 - 20)) /*0x800*/
#define MSR_BIT_SE                                  (1 << (31 - 21)) /*0x400*/
#define MSR_BIT_BE                                  (1 << (31 - 22)) /*0x200*/
#define MSR_BIT_FE1                                 (1 << (31 - 23)) /*0x100*/
#define MSR_BIT_RESERVED1                           (1 << (31 - 24)) /*0x80*/
#define MSR_BIT_IP                                  (1 << (31 - 25)) /*0x40*/
#define MSR_BIT_IR                                  (1 << (31 - 26)) /*0x20*/
#define MSR_BIT_DR                                  (1 << (31 - 27)) /*0x10*/
#define MSR_BIT_RESERVED2                           (1 << (31 - 28)) /*8*/
#define MSR_BIT_PM                                  (1 << (31 - 29)) /*4*/
#define MSR_BIT_RI                                  (1 << (31 - 30)) /*2 */
#define MSR_BIT_LE                                  (1 << (31 - 31)) /*1*/

#define TRAP_INSTRUCTION                            0x7FE00008
#define DBGSTR_INSTRUCTION                          0x7ffff808
#define DBGMSG_INSTRUCTION                          0x7ffef008
#define DBGCTL_INSTRUCTION                          0x7ffe0008

#define GDB_REGISTER_R0                             0x00
#define GDB_REGISTER_R1                             0x01
#define GDB_REGISTER_R2                             0x02
#define GDB_REGISTER_R3                             0x03
#define GDB_REGISTER_R4                             0x04
#define GDB_REGISTER_R5                             0x05
#define GDB_REGISTER_R6                             0x06
#define GDB_REGISTER_R7                             0x07
#define GDB_REGISTER_R31                            0x1F
#define GDB_REGISTER_F0                             0x20
#define GDB_REGISTER_F31                            0x3F
#define GDB_REGISTER_PC                             0x40
#define GDB_REGISTER_MSR                            0x41
#define GDB_REGISTER_CR                             0x42
#define GDB_REGISTER_LR                             0x43
#define GDB_REGISTER_CTR                            0x44
#define GDB_REGISTER_XER                            0x45
#define GDB_REGISTER_FPSCR                          0x46

#define GDB_REGISTER_DAR                            0x6a
#define GDB_REGISTER_DSISR                          0x6b

#define GDB_REGISTER_SRR0                           0x70
#define GDB_REGISTER_SRR1                           0x71
#define GDB_REGISTER_DABR                           0x75
#define GDB_REGISTER_IABR                           0x79


#define GDB_REGISTER_HID2                           0x9F
#define GDB_REGISTER_PIR                            0xD6
#define GDB_REGISTER_WPAR                           0xA5
#define GDB_REGISTER_PS0                            0xA6
#define GDB_REGISTER_PS31                           0xC5
#define GDB_REGISTER_GQR0                           0xC6
#define GDB_REGISTER_GQR1                           0xC7
#define GDB_REGISTER_GQR2                           0xC8
#define GDB_REGISTER_GQR3                           0xC9
#define GDB_REGISTER_GQR4                           0xCA
#define GDB_REGISTER_GQR5                           0xCB
#define GDB_REGISTER_GQR6                           0xCC
#define GDB_REGISTER_GQR7                           0xCD

#define DSISR_FLAG_ACCESS_RESTRICTED_BY_BAT_OR_PAGE 1 << (31 - 4)
#define DSISR_FLAG_DABRMatch                        1 << (31 - 9)