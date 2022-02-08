#include "debugger.h"
#include "exceptions.h"
#include "logger.h"
#include <coreinit/debug.h>
#include <wups.h>

OSThread **pThreadList;


WUPS_PLUGIN_NAME("Debugger");
WUPS_PLUGIN_DESCRIPTION("FTP Server");
WUPS_PLUGIN_VERSION("0.1");
WUPS_PLUGIN_AUTHOR("Kinnay");
WUPS_PLUGIN_LICENSE("GPL");

WUPS_USE_WUT_DEVOPTAB();

INITIALIZE_PLUGIN() {
    InstallExceptionHandlers();
}
ON_APPLICATION_START() {
    initLogging();
    DEBUG_FUNCTION_LINE("Started Debugger plugin");
    pThreadList = (OSThread **) 0x100567F8;
    debugger    = new Debugger();
    DEBUG_FUNCTION_LINE("Created Debugger");
    debugger->start();
    DEBUG_FUNCTION_LINE("Started Debugger thread");
}

ON_APPLICATION_REQUESTS_EXIT() {
    DEBUG_FUNCTION_LINE("Deleting Debugger thread");
    delete debugger;
    DEBUG_FUNCTION_LINE("Deleted Debugger thread");
    deinitLogging();
}
