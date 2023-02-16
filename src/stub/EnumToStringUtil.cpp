#include "EnumToStringUtil.h"

const char *GDBStub_OSExceptionToString(OSExceptionType type) {
    switch (type) {
        case OS_EXCEPTION_TYPE_SYSTEM_RESET:
            return "SYSTEM_RESET";
        case OS_EXCEPTION_TYPE_MACHINE_CHECK:
            return "MACHINE_CHECK";
        case OS_EXCEPTION_TYPE_DSI:
            return "DSI      ";
        case OS_EXCEPTION_TYPE_ISI:
            return "ISI      ";
        case OS_EXCEPTION_TYPE_EXTERNAL_INTERRUPT:
            return "EXTERNAL ";
        case OS_EXCEPTION_TYPE_ALIGNMENT:
            return "ALIGNMENT";
        case OS_EXCEPTION_TYPE_PROGRAM:
            return "PROGRAM  ";
        case OS_EXCEPTION_TYPE_FLOATING_POINT:
            return "FP       ";
        case OS_EXCEPTION_TYPE_DECREMENTER:
            return "DECREMENT";
        case OS_EXCEPTION_TYPE_SYSTEM_CALL:
            return "SYS_CALL ";
        case OS_EXCEPTION_TYPE_TRACE:
            return "TRACE    ";
        case OS_EXCEPTION_TYPE_PERFORMANCE_MONITOR:
            return "PERF_MON ";
        case OS_EXCEPTION_TYPE_BREAKPOINT:
            return "BREAKPTR  ";
        case OS_EXCEPTION_TYPE_SYSTEM_INTERRUPT:
            return "SYS_INTER";
        case OS_EXCEPTION_TYPE_ICI:
            return "ICI      ";
        case 15:
            return "NONE*****";
    }
    return "UNKNOWN EXCEPTION";
}

// Checked
const char *GDBStub_SigToString(GDB_SIGNAL signal) {
    switch (signal) {
        case GDBSIGNAL_SIG0:
            return "GDBSIGNAL_SIG0";
        case GDBSIGNAL_SIGINT:
            return "GDBSIGNAL_SIGINT";
        case GDBSIGNAL_SIGQUIT:
            return "GDBSIGNAL_SIGQUIT";
        case GDBSIGNAL_SIGILL:
            return "GDBSIGNAL_SIGILL";
        case GDBSIGNAL_SIGTRAP:
            return "GDBSIGNAL_SIGTRAP";
        case GDBSIGNAL_SIGKILL:
            return "GDBSIGNAL_SIGKILL";
        case GDBSIGNAL_SIGBUS:
            return "GDBSIGNAL_SIGBUS";
        case GDBSIGNAL_SIGSEGV:
            return "GDBSIGNAL_SIGSEGV";
        case GDBSIGNAL_SIGTERM:
            return "GDBSIGNAL_SIGTERM";
        case GDBSIGNAL_SIGSTOP:
            return "GDBSIGNAL_SIGSTOP";
        case GDBSIGNAL_RESUME:
            return "GDBSIGNAL_RESUME";
        case GDBSIGNAL_DBGCTL:
            return "GDBSIGNAL_DBGCTL";
        case GDBSIGNAL_DBGMSG:
            return "GDBSIGNAL_DBGMSG";
        case GDBSIGNAL_BREAKPT:
            return "GDBSIGNAL_BREAKPT";
        case GDBSIGNAL_DBGSTR:
            return "GDBSIGNAL_DBGSTR";
    }
    return "**INVALID**";
}

// Checked
const char *GDBStub_StateToString(GDBStubState state) {
    switch (state) {
        case GDBSTUB_STATE_INACTIVE:
            return "INACTIVE";
        case GDBSTUB_STATE_STOPPED:
            return "STOPPED";
        case GDBSTUB_STATE_RUNNING:
            return "RUNNING";
        case GDBSTUB_STATE_STOPPING:
            return "STOPPING";
        case GDBSTUB_STATE_SINGLESTEP:
            return "SINGLESTEP";
        case GDBSTUB_STATE_PROCESSING:
            return "PROCESSING";
        case GDBSTUB_STATE_HUNG:
            return "HUNG";
    }
    return "**INVALID**";
}

const char *GDBStub_GDBFlagsToString(GDBStub_GDBFlags flag) {
    switch (flag) {
        case GDBFLAGS_ZERO:
            return "<zero> ";
        case GDBFLAGS_GDBSTOP:
            return "GDBSTOP";
        case GDBFLAGS_GDBSTEP:
            return "GDBSTEP";
        case GDBFLAGS_GDBCONT:
            return "GDBCONT";
        default:
            break;
    }
    return "<inval>";
}

const char *GDBStub_CmdToString(GDBStub_CMD cmd) {
    switch (cmd) {
        case GDBSTUB_CMD_IDLE:
            return "IDLE";
        case GDBSTUB_CMD_STOP:
            return "STOP";
        case GDBSTUB_CMD_CONTINUE:
            return "CONTINUE";
        case GDBSTUB_CMD_SINGLESTEP:
            return "SINGLESTEP";
        case GDBSTUB_CMD_GETSYSREG:
            return "GETSYSREG";
        case GDBSTUB_CMD_PUTSYSREG:
            return "PUTSYSREG";
        case GDBSTUB_CMD_ICINVALIDBLK:
            return "ICINVALIDBLK";
        case GDBSTUB_CMD_DCINVALRNG:
            return "DCINVALRNG";
        case GDBSTUB_CMD_GETCOREMEM:
            return "GETCOREMEM";
        case GDBSTUB_CMD_PUTCOREMEM:
            return "PUTCOREMEM";
    }
    return "**INVALID**";
}

const char *GDBStub_ReqToString(GDBStub_Req req) {
    switch (req) {
        case GDBSTUB_REQ_STOPPED:
            return "STOPPED";
        case GDBSTUB_REQ_FINISHED:
            return "FINISHED";
    }
    return "**INVALID**";
}
