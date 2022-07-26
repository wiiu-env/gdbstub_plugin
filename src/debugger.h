
#pragma once

#include "kernel.h"
#include "socket.h"
#include <coreinit/messagequeue.h>
#include <coreinit/mutex.h>
#include <coreinit/thread.h>

#include <coreinit/exception.h>
#include <cstdint>
#include <vector>

#define MESSAGE_COUNT 10
#define STACK_SIZE    0x8000

#define TRAP          0x7FE00008


extern OSThread **pThreadList;

#define ThreadList (*pThreadList)

template<int N>
class Bits {
public:
    Bits(uint32_t value) {
        this->value = value;
    }

    bool operator[](int index) {
        return (value >> (N - index - 1)) & 1;
    }

private:
    uint32_t value{};
};


class ExceptionState {
public:
    enum Type {
        DSI,
        ISI,
        PROGRAM
    };

    bool isBreakpoint();
    void resume();

    Type type;
    OSContext context{};

    OSThread *thread{};

    OSMessageQueue queue{};
    OSMessage message{};

    bool isPaused{};
};


class ExceptionMgr {
public:
    void init();
    void lock();
    void unlock();
    void cleanup();
    ExceptionState *find(OSThread *thread);
    ExceptionState *findOrCreate(OSThread *thread);

private:
    OSMutex mutex;
    std::vector<ExceptionState *> list;
};


class BreakPoint {
public:
    bool isRange(uint32_t addr, uint32_t length) const;

    uint32_t address;
    uint32_t instruction;
    bool isSpecial;
};

class SpecialBreakPoint : public BreakPoint {
public:
    OSThread *thread;
};


template<class T>
class BreakPointList {
public:
    size_t size() {
        return list.size();
    }

    T *alloc() {
        for (size_t i = 0; i < size(); i++) {
            if (list[i].address == 0) {
                return &list[i];
            }
        }

        T newBp;
        newBp.address     = 0;
        newBp.instruction = 0;
        list.push_back(newBp);
        return &list.back();
    }

    T *find(uint32_t addr) {
        for (size_t i = 0; i < size(); i++) {
            if (list[i].address == addr) {
                return &list[i];
            }
        }
        return nullptr;
    }

    T *findRange(uint32_t addr, uint32_t length, int *index) {
        size_t i = *index;
        while (i < size()) {
            if (list[i].isRange(addr, length)) {
                *index = (int) i + 1;
                return &list[i];
            }
            i++;
        }

        if ((int) i > *index) {
            *index = (int) i;
        }
        return nullptr;
    }

    T *operator[](int index) {
        return &list[index];
    }

    void cleanup() {
        for (size_t i = 0; i < size(); i++) {
            if (list[i].address != 0) {
                KernelWriteU32(list[i].address, list[i].instruction);
                list[i].address     = 0;
                list[i].instruction = 0;
            }
        }
    }

private:
    std::vector<T> list;
};


class BreakPointMgr {
public:
    void init();
    void lock();
    void unlock();
    void cleanup();
    bool isCustom(uint32_t addr);
    bool isSoftware(uint32_t addr);
    bool isSpecial(uint32_t addr);
    void read(void *buffer, uint32_t addr, uint32_t length);
    void write(const void *buffer, uint32_t addr, uint32_t length);
    void toggle(uint32_t addr);
    uint32_t getInstr(uint32_t addr);
    BreakPoint *find(uint32_t addr, bool includeSpecial);
    BreakPoint *findRange(uint32_t addr, uint32_t length, int *index, bool includeSpecial);
    SpecialBreakPoint *findSpecial(uint32_t addr, OSThread *thread);
    void clearSpecial(OSThread *thread);
    void predictStep(ExceptionState *state, bool stepOver);

private:
    BreakPoint *alloc();
    SpecialBreakPoint *allocSpecial();
    void disable(BreakPoint *bp);
    void enable(BreakPoint *bp, uint32_t addr);

    BreakPointList<BreakPoint> breakpoints;
    BreakPointList<SpecialBreakPoint> special;

    OSMutex mutex{};
};


class StepMgr {
public:
    void init();
    void lock();
    void unlock();
    void singleStep(ExceptionState *state, uint32_t instruction);
    void handleBreakPoint(ExceptionState *state);
    void adjustAddress(ExceptionState *state);

private:
    static uint32_t buffer[96];

    uint32_t *alloc();
    void free(int index);
    static void branchConditional(ExceptionState *state, uint32_t instruction, uint32_t target, bool checkCtr);

    OSMutex mutex;

    uint32_t usedMask;
};


class Debugger {
public:
    enum StepCommand {
        STEP_CONTINUE,
        STEP_INTO,
        STEP_OVER
    };

    void start();

    ~Debugger();

private:
    enum Command {
        COMMAND_CLOSE,
        COMMAND_READ,
        COMMAND_WRITE,
        COMMAND_WRITE_CODE,
        COMMAND_GET_MODULE_NAME,
        COMMAND_GET_MODULE_LIST,
        COMMAND_GET_THREAD_LIST,
        COMMAND_GET_STACK_TRACE,
        COMMAND_TOGGLE_BREAKPOINT,
        COMMAND_POKE_REGISTERS,
        COMMAND_RECEIVE_MESSAGES,
        COMMAND_SEND_MESSAGE,
        COMMAND_DISASM
    };

    static int threadEntry(int argc, const char **argv);
    static BOOL dsiHandler(OSContext *context);
    static BOOL isiHandler(OSContext *context);
    static BOOL programHandler(OSContext *context);
    static void exceptionHandler(OSContext *context, ExceptionState::Type type);

    void threadFunc();
    void mainLoop(Client *client);
    void handleException(OSContext *context, ExceptionState::Type type);
    void handleFatalCrash(OSContext *context, ExceptionState::Type type);
    void handleCrash(ExceptionState *state);
    void handleBreakPoint(ExceptionState *state);
    void processBreakPoint(ExceptionState *state);
    void resumeBreakPoint(ExceptionState *state);
    StepCommand notifyBreak(ExceptionState *state);
    void cleanup();

    bool checkDataRead(uint32_t addr, uint32_t length);

    OSMessageQueue eventQueue{};
    OSMessage eventMessages[MESSAGE_COUNT]{};

    OSThread *serverThread{};
    char *serverStack{};

    BreakPointMgr breakpoints;
    ExceptionMgr exceptions;
    StepMgr stepper{};

    bool stopRunning = false;
    bool initialized{};
    bool connected{};
    bool firstTrap{};

    OSExceptionCallbackFn prevDsiHandler = nullptr;
    OSExceptionCallbackFn prevIsiHandler = nullptr;
    OSExceptionCallbackFn prevProgramHandler = nullptr;
};

extern "C" Debugger *debugger;
extern bool initDebugState;
