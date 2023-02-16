#pragma once
#include "GDBDefines.h"
#include <coreinit/exception.h>
#include <cstdint>

void ResetIOOffsets();

// The MasterAgent_MainLoop still has a fixed buffer we need to replace before we
// can increase this value
#define IO_BUFFER_CAPACITY           0x800

#define MasterAgent_IOInit           ((void (*)(void))(0x101C400 + 0x02039158 - 0x02000000))
#define MasterAgent_IOPutString      ((void (*)(const char *str))(0x101C400 + 0x0203b6b4 - 0x02000000))
#define MasterAgent_IOSendEx         ((void (*)(bool))(0x101C400 + 0x02039a28 - 0x02000000))
#define MasterAgent_PutPacket        ((void (*)(const char *packetStr, bool checkPendingInputFirst))(0x101C400 + 0x020394d8 - 0x02000000))
#define MasterAgent_IOPutStringAsHex ((void (*)(const char *str))(0x101C400 + 0x0203a81c - 0x02000000))
#define MasterAgent_SendResult       ((void (*)(uint32_t))(0x101C400 + 0x0203bafc - 0x02000000))
#define MasterAgent_PutPacket        ((void (*)(const char *packetStr, bool checkPendingInputFirst))(0x101C400 + 0x020394d8 - 0x02000000))
#define MasterAgent_IOInit           ((void (*)(void))(0x101C400 + 0x02039158 - 0x02000000))
#define MasterAgent_IOPutString      ((void (*)(const char *str))(0x101C400 + 0x0203b6b4 - 0x02000000))
#define MasterAgent_IOPutHex32       ((void (*)(uint32_t value))(0x101C400 + 0x02039190 - 0x02000000))
#define MasterAgent_IOWriteString    ((void (*)(const char *))(0x101C400 + 0x020394a8 - 0x02000000))
#define MasterAgent_IOPutHex64       ((void (*)(uint64_t))(0x101C400 + 0x0203b6f8 - 0x02000000))
#define gdb_mem2hex                  ((uint32_t(*)(uint32_t addr, uint32_t size))(0x101C400 + 0x0203bcec - 0x02000000))
#define Waikiki_Read                 ((int32_t(*)(char *buffer, uint32_t bufferSize))(0x101C400 + 0x0203f2f0 - 0x02000000))