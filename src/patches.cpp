#include <cstdint>
#include <wups.h>

DECL_FUNCTION(int, OSSetExceptionCallback) {
    return 0;
}
DECL_FUNCTION(int, OSSetExceptionCallbackEx) {
    return 0;
}
DECL_FUNCTION(int, OSIsDebuggerInitialized) {
    return true;
}


WUPS_MUST_REPLACE(OSSetExceptionCallback, WUPS_LOADER_LIBRARY_COREINIT, OSSetExceptionCallback);
WUPS_MUST_REPLACE(OSSetExceptionCallbackEx, WUPS_LOADER_LIBRARY_COREINIT, OSSetExceptionCallbackEx);
WUPS_MUST_REPLACE(OSIsDebuggerInitialized, WUPS_LOADER_LIBRARY_COREINIT, OSIsDebuggerInitialized);