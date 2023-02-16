#pragma once

#include "GDBDefines.h"
#include <coreinit/exception.h>

const char *GDBStub_OSExceptionToString(OSExceptionType type);
const char *GDBStub_SigToString(GDB_SIGNAL signal);
const char *GDBStub_StateToString(GDBStubState state);
const char *GDBStub_GDBFlagsToString(GDBStub_GDBFlags flag);
const char *GDBStub_CmdToString(GDBStub_CMD cmd);
const char *GDBStub_ReqToString(GDBStub_Req req);
