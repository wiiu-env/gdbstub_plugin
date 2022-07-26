#include "logger.h"
#include <coreinit/exception.h>
#include <cstdint>
#include <wups.h>

extern bool initDebugState;

DECL_FUNCTION(OSExceptionCallbackFn, OSSetExceptionCallback,
              OSExceptionType exceptionType,
              OSExceptionCallbackFn callback) {
    if (initDebugState) {
        return nullptr;
    } else {
        return real_OSSetExceptionCallback(exceptionType, callback);
    }
}
DECL_FUNCTION(OSExceptionCallbackFn, OSSetExceptionCallbackEx,
              OSExceptionMode mode,
              OSExceptionType exceptionType,
              OSExceptionCallbackFn callback) {
    if (initDebugState) {
        return nullptr;
    } else {
        return real_OSSetExceptionCallbackEx(mode, exceptionType, callback);
    }
}
DECL_FUNCTION(int, OSIsDebuggerInitialized) {
    return initDebugState;
}

// OSSetExceptionCallbackEx is just a branch to another function.
// Instead of "fixing" the FunctionPatcher, we use this hacky solution :)
WUPS_MUST_REPLACE_PHYSICAL_FOR_PROCESS(OSSetExceptionCallbackEx, (0x3201C400 + 0x286b0), (0x101C400 + 0x286b0), WUPS_FP_TARGET_PROCESS_GAME_AND_MENU);
WUPS_MUST_REPLACE_FOR_PROCESS(OSSetExceptionCallback, WUPS_LOADER_LIBRARY_COREINIT, OSSetExceptionCallback, WUPS_FP_TARGET_PROCESS_GAME_AND_MENU);
WUPS_MUST_REPLACE_FOR_PROCESS(OSIsDebuggerInitialized, WUPS_LOADER_LIBRARY_COREINIT, OSIsDebuggerInitialized, WUPS_FP_TARGET_PROCESS_GAME_AND_MENU);
