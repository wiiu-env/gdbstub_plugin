#include "GDB_Commands.h"
#include "GDBDParseUtils.h"
#include "GDBThreadDefines.h"
#include "GDBUtils.h"
#include "GDB_IO.h"
#include "MasterAgent_RegMemReadWrite.h"
#include "MasterAgent_ThreadUtils.h"
#include "imports.h"
#include "utils/IOUtils.h"
#include <cerrno>
#include <coreinit/debug.h>
#include <coreinit/dynload.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string_view>
#include <wups/function_patching.h>

static char sGDBQueryStringBuffer[100] = {};

static char sOutPaginationBuffer[IO_BUFFER_CAPACITY - 4] = {};

void qSupported(const char *packet) {
    gIsDebuggerPresent = 1;
#ifdef SHOW_DRIVER_THREADS
    gGDBGHSShowDrvrThreads = 1;
#endif
    DAT_100d1378                = DAT_100d1378 + 1;
    gTimerKernelCallbackLastRun = in_TBLr() + g100msInTicks;
    MasterAgent_IOInit();
    MasterAgent_IOPutString("PacketSize=");
    snprintf(sGDBQueryStringBuffer, sizeof(sGDBQueryStringBuffer), "%x", IO_BUFFER_CAPACITY);
    MasterAgent_IOPutString(sGDBQueryStringBuffer);
    MasterAgent_IOPutString(";qXfer:features:read+;qXfer:threads:read+;swbreak+;hwbreak+");
    MasterAgent_IOPutString(";COSVer=");
    snprintf(sGDBQueryStringBuffer, sizeof(sGDBQueryStringBuffer), "%d", 21301);
    MasterAgent_IOPutString(sGDBQueryStringBuffer);
    MasterAgent_IOSendEx(false);
}

void qOffsets(const char *packet) {
    OSDynLoad_InternalData *rplIterator = *(reinterpret_cast<OSDynLoad_InternalData **>(0x10081018));
    OSDynLoad_NotifyData *rpl           = nullptr;

    for (; rplIterator != nullptr; rplIterator = rplIterator->next) {
        if (!rplIterator->notifyData->name || std::string_view(rplIterator->notifyData->name).ends_with("rpx")) {
            rpl = rplIterator->notifyData;
            break;
        }
    }

    if (rpl != nullptr) {
        MasterAgent_IOInit();

        snprintf(sGDBQueryStringBuffer, sizeof(sGDBQueryStringBuffer), "TextSeg=%08X;DataSeg=%08X", rpl->textAddr, rpl->dataAddr);
        MasterAgent_IOPutString(sGDBQueryStringBuffer);
        MasterAgent_IOSendEx(false);
    } else {
        MasterAgent_PutPacket("", false);
    }
}

static char sLibrariesReadBuffer[IO_BUFFER_CAPACITY] = {};

bool qXferLibrariesRead(const char *packet) {
    char *charPtr        = (char *) packet;
    uint32_t read_offset = 0;
    uint32_t read_length = 0;
    if (charPtr[0] != '\0') {
        read_offset = gdb_getint(&charPtr);
        charPtr++;
    } else {
        OSReport("WARNING: qXferLibrariesRead failed to read offset.\n");
    }
    if (charPtr[0] != '\0') {
        read_length = gdb_getint(&charPtr);
    } else {
        OSReport("WARNING: qXferLibrariesRead failed to read length.\n");
    }

    if (((read_length * 2) + 1) > (IO_BUFFER_CAPACITY - 4)) {
        read_length = ((IO_BUFFER_CAPACITY - 4) / 2) - 1;
    }
    MasterAgent_IOInit();

    if (GDBqXfer_libraries_read(read_offset, read_length + 1, sLibrariesReadBuffer) == 0) {
        MasterAgent_IOPutString("l");
        DAT_100d1374 = 0;
    } else {
        MasterAgent_IOPutString("m");
    }
    MasterAgent_IOPutStringAsHex(sLibrariesReadBuffer);
    MasterAgent_IOSendEx(false);
    return true;
}

bool qXferFeaturesRead(const char *packet) {
    auto *annexPtr = (char *) packet;
    char *annexEnd = annexPtr;
    while (annexEnd[0] != '\0' && annexEnd[0] != ':') {
        annexEnd++;
    }
    annexEnd[0]   = '\0';
    auto *charPtr = annexEnd + 1;
    if (strncmp(annexPtr, "target.xml", 10) == 0) {
        uint32_t read_offset = 0;
        uint32_t read_length = 0;
        if (charPtr[0] != '\0') {
            read_offset = gdb_getint(&charPtr);
            charPtr++;
        } else {
            OSReport("WARNING: qXferFeaturesRead failed to read offset.\n");
        }
        if (charPtr[0] != '\0') {
            read_length = gdb_getint(&charPtr);
        } else {
            OSReport("WARNING: qXferFeaturesRead failed to read length.\n");
        }
        if (read_length == 0 || read_offset >= GDBTargetXML.size()) {
            MasterAgent_PutPacket("l", false);
        } else {
            auto paginated_str = GDBTargetXML.substr(read_offset, read_length);
            memset(sOutPaginationBuffer, 0, sizeof(sOutPaginationBuffer));
            if (paginated_str.size() < sizeof(sOutPaginationBuffer)) {
                memcpy(sOutPaginationBuffer, paginated_str.data(), paginated_str.size());
            } else {
                OSReport("WARN: qXferFeaturesRead tried to overflow sOutPaginationBuffer!\n");
            }
            MasterAgent_IOInit();
            MasterAgent_IOPutString((paginated_str.size() == read_length) ? "m" : "l");
            MasterAgent_IOPutString(sOutPaginationBuffer);
            MasterAgent_IOSendEx(false);
        }
        return true;
    }
    return false;
}

void qXferThreadRead(const char *packet) {
    auto *charPtr        = (char *) packet;
    uint32_t read_offset = 0;
    uint32_t read_length = 0;
    if (charPtr[0] != '\0') {
        read_offset = gdb_getint(&charPtr);
        charPtr++;
    } else {
        OSReport("WARNING: qXferThreadRead failed to read offset.\n");
    }
    if (charPtr[0] != '\0') {
        read_length = gdb_getint(&charPtr);
    } else {
        OSReport("WARNING: qXferThreadRead failed to read length.\n");
    }
    if (read_length == 0) {
        MasterAgent_PutPacket("l", false);
        return;
    }

    int32_t index = 0;
    InitThreadInfoBuffer();
    PutStringIntoThreadInfoBuffer(R"(<?xml version="1.0"?> <threads>)");
    while (true) {
        auto *thread = MasterAgent_GetIndexedThreadID(index);
        if (thread == (OSThread *) 0xFFFFFFFF) {
            break;
        }

        auto threadID = MasterAgent_GetThreadID(thread);
        uint32_t core;
        const char *threadName;
        char threadNameOnStack[0x18];

        if (thread == IDLE_THREAD_CORE_0) {
            core       = 0;
            threadName = "Idle Core 0";
        } else if (thread == IDLE_THREAD_CORE_1) {
            core       = 1;
            threadName = "Idle Core 1";
        } else if (thread == IDLE_THREAD_CORE_2) {
            core       = 2;
            threadName = "Idle Core 2";
        } else {
            core       = thread->context.upir;
            threadName = thread->name;
            if (!threadName) {
                snprintf(threadNameOnStack, 0x18, "Thread 0x%8.8x", (uint32_t) thread);
                threadName = threadNameOnStack;
            }
        }
        snprintf(sGDBQueryStringBuffer, sizeof(sGDBQueryStringBuffer), R"(<thread id="%X" core="%d" name=")", threadID, core);
        PutStringIntoThreadInfoBuffer(sGDBQueryStringBuffer);

        PutXMLEscapedStringIntoThreadInfoBuffer(threadName);
        PutStringIntoThreadInfoBuffer("\">");

        PutXMLEscapedStringIntoThreadInfoBuffer(MasterAgent_ThreadExtraInfo(thread));

        PutStringIntoThreadInfoBuffer("</thread>");

        index++;
    }
    PutStringIntoThreadInfoBuffer("</threads>");
    std::string_view resulBufAsView(GetThreadInfoBuffer());
    if (read_offset >= resulBufAsView.size()) {
        MasterAgent_PutPacket("l", false);
    } else {
        auto paginated_str = resulBufAsView.substr(read_offset, read_length);
        memset(sOutPaginationBuffer, 0, sizeof(sOutPaginationBuffer));
        if (paginated_str.size() < sizeof(sOutPaginationBuffer)) {
            memcpy(sOutPaginationBuffer, paginated_str.data(), paginated_str.size());
        } else {
            OSReport("WARN: qXferThreadRead tried to overflow sOutPaginationBuffer!\n");
        }
        MasterAgent_IOInit();
        MasterAgent_IOPutString((paginated_str.size() == read_length) ? "m" : "l");
        MasterAgent_IOPutString(sOutPaginationBuffer);
        MasterAgent_IOSendEx(false);
    }
}

void gdb_vpacket(const char *packet) {
    auto packetAsString = (char *) packet;

    if (strcmp(packetAsString, "vCont?") == 0) {
        MasterAgent_PutPacket("vCont;c;C;s;S;", false);
        return;
    }
    if (strcmp(packetAsString, "vCont;c") == 0) {
        gdb_continue("c");
        return;
    }
    if (strncmp(packetAsString, "vGetRegs;", 9) == 0) {
        gdb_vgetregisters(&packetAsString[9]);
        return;
    }
    if (strncmp(packetAsString, "vCont;", 6) != 0) {
        MasterAgent_PutPacket("", false);
        return;
    }
    packetAsString = &packetAsString[6];
    for (uint32_t i = 0; i < 3; i++) {
        gCoreData[i].flag         = GDBFLAGS_GDBSTOP;
        gCoreData[i].nextSignal__ = GDBSIGNAL_SIG0;
    }

    bool stepDone = false;

    uint32_t result = 0;
    while (true) {
        if (packetAsString[0] == '\0') {
            if (result == 0) {
                gGDBStub_main_state = GDBMAINSTATE_VCONT;
            }
            MasterAgent_SendResult(result);
            return;
        }
        auto packetType = packetAsString[0];
        if (packetType == 'C' || packetType == 'c' || packetType == 'S' || packetType == 's') {
            packetAsString++;
            GDB_SIGNAL signal = GDBSIGNAL_SIG0;
            GDBStub_GDBFlags flag;

            if (packetType == 'C' || packetType == 'S') {
                signal = (GDB_SIGNAL) gdb_getint(&packetAsString);
            }

            if (packetType == 'C' || packetType == 'c') {
                if (!stepDone) {
                    flag = GDBFLAGS_GDBCONT;
                } else {
                    flag = GDBFLAGS_GDBSTOP;
                }
            } else {
                flag = GDBFLAGS_GDBSTEP;
            }

            if (packetAsString[0] == '\0' || packetAsString[0] == ';') {
                for (uint32_t i = 0; i < gCoreCount; i++) {
                    if (gCoreData[i].flag == GDBFLAGS_GDBSTOP) {
                        gCoreData[i].signal = GDBSIGNAL_RESUME;
                        if (flag == GDBFLAGS_GDBSTEP) {
                            stepDone = true;
                        }
                        gCoreData[i].flag         = flag;
                        gCoreData[i].nextSignal__ = signal;
                    }
                }
            } else if (packetAsString[0] == ':') {
                packetAsString++;
                auto *thread = gdb_get_pid_tid(&packetAsString);
                bool found   = false;
                for (uint32_t i = 0; i < gCoreCount; i++) {
                    auto *coreDataForI = &gCoreData[i];
                    if (coreDataForI->stoppedThread == thread) {
                        coreDataForI->signal = GDBSIGNAL_RESUME;
                        if (flag == GDBFLAGS_GDBSTEP) {
                            stepDone = true;
                        }
                        coreDataForI->flag         = flag;
                        coreDataForI->nextSignal__ = signal;
                        auto MSR                   = MasterAgent_GetRegister32(coreDataForI, coreDataForI->stoppedContext, GDB_REGISTER_MSR);
                        if ((MSR & MSR_BIT_SE) != 0) {
                            result = MasterAgent_SetRegister32(coreDataForI, coreDataForI->stoppedContext, GDB_REGISTER_MSR, MSR & ~MSR_BIT_SE);
                        }
                        found = true;
                    }
                }

                if (!found) {
                    OSReport("WARN: gdb_vpacket Core which runs thread %08x not found\n", thread);
                    MasterAgent_SendResult(EPERM);
                    return;
                }
            }
        } else {
            OSReport("WARN: gdb_vpacket invalid packet type. %s\n", packetAsString);
            MasterAgent_SendResult(EINVAL);
            return;
        }
        if (packetAsString[0] == ';') {
            packetAsString++;
        }
    }
}

static bool gdb_sendmemory(const char *packet) {
    auto *intPtr = (char *) packet;
    auto addr    = gdb_getint(&intPtr);
    if (intPtr[0] != ',') {
        MasterAgent_SendResult(EINVAL);
        return true;
    }
    intPtr++;
    auto size = gdb_getint(&intPtr);
    MasterAgent_IOInit();
    auto readResult = gdb_mem2hex(addr, size);
    if (readResult == 0) {
        MasterAgent_IOSendEx(false);
    } else {
        MasterAgent_SendResult(EIO);
    }

    return true;
}

static void gdb_sendregister(const char *packet) {
    auto *intPtr = (char *) (packet);
    auto regId   = gdb_getint(&intPtr);
    if (regId > 0xdb) {
        MasterAgent_SendResult(EINVAL);
        return;
    }
    MasterAgent_IOInit();
    if ((regId >= GDB_REGISTER_F0 && regId <= GDB_REGISTER_F31) ||
        (regId >= GDB_REGISTER_PS0 && regId <= GDB_REGISTER_PS31)) {
        auto res = MasterAgent_GetRegister64(gGDBCurCoreInfo, gGDBCurCoreInfo->currentContext, regId);

        MasterAgent_IOPutHex64(res);
    } else {
        auto res = MasterAgent_GetRegister32(gGDBCurCoreInfo, gGDBCurCoreInfo->currentContext, regId);
        MasterAgent_IOPutHex32(res);
    }
    MasterAgent_IOSendEx(false);
}

DECL_FUNCTION(void, MasterAgent_ProcessPacket, const char *packet) {
#ifdef PACKET_LOGGING
    OSReport("Process packet \"%s\"\n", packet);
#endif
    auto prefix = packet[0];
    if (prefix == 'v') {
        gdb_vpacket(packet);
        return;
    }
    if (prefix == 'm') {
        gdb_sendmemory(&packet[1]);
        return;
    } else if (prefix == 'p') {
        gdb_sendregister(&packet[1]);
        return;
    } else if (prefix == 'q') {
        auto *charPtr = (char *) (&packet[1]);
        if (strncmp(charPtr, "Supported:", 10) == 0) {
            qSupported(&charPtr[10]);
            return;
        } else if (strncmp(charPtr, "Xfer:", 5) == 0) {
            charPtr = (char *) (&charPtr[5]);
            if (strncmp(charPtr, "threads:read::", 14) == 0) {
                qXferThreadRead(&charPtr[14]);
                return;
            } else if (strncmp(charPtr, "features:read:", 14) == 0) {
                if (qXferFeaturesRead(&charPtr[14])) {
                    return;
                }
            } else if (strncmp(charPtr, "libraries:read::", 16) == 0) {
                if (qXferLibrariesRead(&charPtr[16])) {
                    return;
                }
            }
        } else if (strncmp(charPtr, "Offsets", 7) == 0) {
            qOffsets(&charPtr[7]);
            return;
        }
    }

    real_MasterAgent_ProcessPacket(packet);
}

WUPS_MUST_REPLACE_PHYSICAL_FOR_PROCESS(MasterAgent_ProcessPacket, (0x3201C400 + (0x0203e4c4 - 0x02000000)), (0x101C400 + (0x0203e4c4 - 0x02000000)), WUPS_FP_TARGET_PROCESS_ALL);