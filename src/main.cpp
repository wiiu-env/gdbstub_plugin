#include "main.h"
#include "global_vars.h"
#include "stub/GDB_IO.h"
#include "utils/kernel.h"
#include <coreinit/debug.h>
#include <coreinit/launch.h>
#include <wups.h>
#include <wups_backend/api.h>

WUPS_PLUGIN_NAME("GDBStub");
WUPS_PLUGIN_DESCRIPTION("GDBStub");
WUPS_PLUGIN_VERSION(PLUGIN_VERSION_FULL);
WUPS_PLUGIN_AUTHOR("Maschell, GaryOderNichts");
WUPS_PLUGIN_LICENSE("GPL");

INITIALIZE_PLUGIN() {
    gInitStubs = true;
    EnableIABRandDABRSupport();
    PluginBackendApiErrorType res;
    if ((res = WUPSBackend_InitLibrary()) != PLUGIN_BACKEND_API_ERROR_NONE) {
        OSReport("WUPSBackend_InitLibrary failed: %s", WUPSBackend_GetStatusStr(res));
    }
}

DEINITIALIZE_PLUGIN() {
    // The plugin is not loaded the syscalls would point to invalid memory.
    // So we have to restore the syscalls before unloading
    DisableIABRandDABRSupport();
}

ON_APPLICATION_START() {
    ResetIOOffsets();
    if (!gHeartbeatDisabled) {
        OSReport("Heartbeat not disabled!\n");
        OSRestartGame(0, nullptr);
    }
}

ON_APPLICATION_ENDS() {
    gHeartbeatDisabled         = false;
    bool stubNeedsToBeDisabled = false;
    PluginBackendApiErrorType res;
    if ((res = WUPSBackend_WillReloadPluginsOnNextLaunch(&stubNeedsToBeDisabled)) == PLUGIN_BACKEND_API_ERROR_NONE) {
        if (stubNeedsToBeDisabled) {
            gInitStubs = false;
        }
    } else {
        OSReport("gdbstub: WUPSBackend_WillReloadPluginsOnNextLaunch failed: %s", WUPSBackend_GetStatusStr(res));
    }
}
