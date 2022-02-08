#include "debugger.h"
#include "exceptions.h"
#include "input.h"
#include "logger.h"
#include "screen.h"
#include <coreinit/cache.h>
#include <coreinit/debug.h>
#include <coreinit/dynload.h>
#include <coreinit/exception.h>
#include <coreinit/memory.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <malloc.h>
#include <vpad/input.h>

Debugger *debugger;

bool BreakPoint::isRange(uint32_t addr, uint32_t length) const {
    return address >= addr && address <= addr + length - 1;
}


BreakPoint *BreakPointMgr::find(uint32_t addr, bool includeSpecial) {
    BreakPoint *bp = breakpoints.find(addr);
    if (!bp && includeSpecial) {
        bp = special.find(addr);
    }
    return bp;
}

BreakPoint *BreakPointMgr::findRange(uint32_t addr, uint32_t length, int *index, bool includeSpecial) {
    BreakPoint *bp = breakpoints.findRange(addr, length, index);
    if (bp) {
        return bp;
    }

    if (includeSpecial) {
        int temp = *index - breakpoints.size();
        bp       = special.findRange(addr, length, &temp);
        *index   = temp + breakpoints.size();
        return bp;
    }

    return nullptr;
}

SpecialBreakPoint *BreakPointMgr::findSpecial(uint32_t addr, OSThread *thread) {
    int index             = 0;
    SpecialBreakPoint *bp = special.findRange(addr, 4, &index);
    while (bp) {
        if (bp->thread == thread) {
            return bp;
        }
        bp = special.findRange(addr, 4, &index);
    }
    return nullptr;
}

bool BreakPointMgr::isCustom(uint32_t addr) {
    return find(addr, false);
}

bool BreakPointMgr::isSpecial(uint32_t addr) {
    return find(addr, true) && !find(addr, false);
}

bool BreakPointMgr::isSoftware(uint32_t addr) {
    uint32_t instr  = getInstr(addr);
    uint32_t opcode = instr >> 26;
    if (opcode == 3) { //twi
        return true;
    } else if (opcode == 31) {
        return (instr & 0x7FF) == 8; //tw
    }
    return false;
}

void BreakPointMgr::disable(BreakPoint *bp) {
    uint32_t address = bp->address;
    if (bp->address) {
        bp->address = 0;
        if (!find(address, true)) {
            KernelWriteU32(address, bp->instruction);
        }
        bp->instruction = 0;
    }
}

void BreakPointMgr::enable(BreakPoint *bp, uint32_t addr) {
    BreakPoint *other = find(addr, true);
    if (other) {
        bp->instruction = other->instruction;
    } else {
        bp->instruction = *(uint32_t *) addr;
        KernelWriteU32(addr, TRAP);
    }
    bp->address = addr;
}

void BreakPointMgr::init() {
    OSInitMutex(&mutex);
}

void BreakPointMgr::lock() {
    OSLockMutex(&mutex);
}

void BreakPointMgr::unlock() {
    OSUnlockMutex(&mutex);
}

void BreakPointMgr::cleanup() {
    lock();
    breakpoints.cleanup();
    special.cleanup();
    unlock();
}

void BreakPointMgr::read(void *buffer, uint32_t addr, uint32_t length) {
    lock();

    memcpy(buffer, (void *) addr, length);

    int index      = 0;
    BreakPoint *bp = findRange(addr, length, &index, true);
    while (bp) {
        uint32_t offset = bp->address - addr;
        char *bufptr    = (char *) buffer + offset;
        if (bp->address > addr + length - 4) {
            uint32_t value = bp->instruction;
            for (uint32_t i = 0; i < length - offset; i++) {
                bufptr[i] = value >> 24;
                value <<= 8;
            }
        } else {
            *(uint32_t *) bufptr = bp->instruction;
        }
        bp = findRange(addr, length, &index, true);
    }
    unlock();
}

void BreakPointMgr::write(const void *buffer, uint32_t addr, uint32_t length) {
    lock();

    length = length & ~3;

    int index = 0;
    if (!findRange(addr, length, &index, true)) {
        KernelWrite(addr, buffer, length);
    } else {
        for (uint32_t i = 0; i < length; i += 4) {
            uint32_t value = *(uint32_t *) ((char *) buffer + i);

            int index      = 0;
            BreakPoint *bp = findRange(addr + i, 4, &index, true);
            if (!bp) {
                KernelWriteU32(addr + i, value);
            } else {
                while (bp) {
                    bp->instruction = value;
                    bp              = findRange(addr + i, 4, &index, true);
                }
            }
        }
    }

    unlock();
}

void BreakPointMgr::toggle(uint32_t addr) {
    lock();

    BreakPoint *bp = find(addr, false);
    if (bp) {
        disable(bp);
    } else {
        BreakPoint *bp = breakpoints.alloc();
        bp->isSpecial  = false;
        enable(bp, addr);
    }

    unlock();
}

uint32_t BreakPointMgr::getInstr(uint32_t addr) {
    lock();
    uint32_t instruction;
    BreakPoint *bp = find(addr, true);
    if (bp) {
        instruction = bp->instruction;
    } else {
        instruction = *(uint32_t *) addr;
    }
    unlock();
    return instruction;
}

void BreakPointMgr::clearSpecial(OSThread *thread) {
    lock();
    for (size_t i = 0; i < special.size(); i++) {
        SpecialBreakPoint *bp = special[i];
        if (bp->thread == thread) {
            disable(bp);
        }
    }
    unlock();
}

void BreakPointMgr::predictStep(ExceptionState *state, bool stepOver) {
    lock();

    uint32_t address     = state->context.srr0;
    uint32_t instruction = getInstr(address);

    uint32_t target1 = address + 4;
    uint32_t target2 = 0;

    uint8_t opcode = instruction >> 26;
    if (opcode == 18) { //Branch
        bool AA = instruction & 2;
        bool LK = instruction & 1;

        uint32_t LI = instruction & 0x3FFFFFC;
        if (LI & 0x2000000) LI -= 0x4000000;

        if (!LK || !stepOver) {
            if (AA) target1 = LI;
            else {
                target1 = address + LI;
            }
        }
    }

    else if (opcode == 16) { //Conditional branch
        bool AA = instruction & 2;
        bool LK = instruction & 1;

        uint32_t BD = instruction & 0xFFFC;
        if (BD & 0x8000) BD -= 0x10000;

        if (!LK || !stepOver) {
            if (AA) target2 = BD;
            else {
                target2 = address + BD;
            }
        }
    }

    else if (opcode == 19) { //Conditional branch to lr/ctr
        uint16_t XO = (instruction >> 1) & 0x3FF;
        bool LK     = instruction & 1;

        if (!LK || !stepOver) {
            if (XO == 16) target2 = state->context.lr;
            else if (XO == 528)
                target2 = state->context.ctr;
        }
    }

    SpecialBreakPoint *bp = special.alloc();
    bp->isSpecial         = true;
    bp->thread            = state->thread;
    enable(bp, target1);

    if (target2) {
        SpecialBreakPoint *bp = special.alloc();
        bp->isSpecial         = true;
        bp->thread            = state->thread;
        enable(bp, target2);
    }

    unlock();
}


bool ExceptionState::isBreakpoint() {
    return type == PROGRAM && context.srr1 & 0x20000;
}

void ExceptionState::resume() {
    DEBUG_FUNCTION_LINE("OSLoadContext");
    OSLoadContext(&context);
}


void ExceptionMgr::init() {
    OSInitMutex(&mutex);
}

void ExceptionMgr::lock() {
    OSLockMutex(&mutex);
}

void ExceptionMgr::unlock() {
    OSUnlockMutex(&mutex);
}

void ExceptionMgr::cleanup() {
    OSMessage message;
    message.message = (void *) Debugger::STEP_CONTINUE;

    lock();

    for (auto state : list) {
        if (state->isPaused) {
            OSSendMessage(&state->queue, &message, OS_MESSAGE_FLAGS_NONE);
        }
    }

    unlock();
}

ExceptionState *ExceptionMgr::find(OSThread *thread) {
    lock();
    for (auto state : list) {
        if (state->thread == thread) {
            unlock();
            return state;
        }
    }
    unlock();
    return nullptr;
}

ExceptionState *ExceptionMgr::findOrCreate(OSThread *thread) {
    lock();
    ExceptionState *state = find(thread);
    if (!state) {
        state           = new ExceptionState();
        state->thread   = thread;
        state->isPaused = false;
        OSInitMessageQueue(&state->queue, &state->message, 1);
        list.push_back(state);
    }
    unlock();
    return state;
}


uint32_t StepMgr::buffer[96];

void StepMgr::init() {
    OSInitMutex(&mutex);
    usedMask = 0;
}

void StepMgr::lock() {
    OSLockMutex(&mutex);
}

void StepMgr::unlock() {
    OSUnlockMutex(&mutex);
}

uint32_t *StepMgr::alloc() {
    lock();
    int index     = 0;
    uint32_t mask = usedMask;
    while (mask & 1) {
        mask >>= 1;
        index++;
    }
    usedMask |= 1 << index;
    unlock();

    if (index == 32) {
        OSFatal("Displaced step allocation failed");
    }
    return &buffer[index * 3];
}

void StepMgr::free(int index) {
    lock();
    usedMask &= ~(1 << index);
    unlock();
}

void StepMgr::branchConditional(ExceptionState *state, uint32_t instruction, uint32_t target, bool checkCtr) {
    Bits<5> BO  = (instruction >> 21) & 0x1F;
    Bits<32> CR = state->context.cr;

    uint32_t BI = (instruction >> 16) & 0x1F;
    bool LK     = instruction & 1;

    if (LK) {
        state->context.lr = state->context.srr0 + 4;
    }

    bool ctr_ok = true;
    if (checkCtr) {
        if (!BO[2]) {
            state->context.ctr--;
        }
        ctr_ok = BO[2] || ((state->context.ctr != 0) ^ BO[3]);
    }

    bool cond_ok = BO[0] || (CR[BI] == BO[1]);
    if (ctr_ok && cond_ok) {
        state->context.srr0 = target;
    }
}

void StepMgr::singleStep(ExceptionState *state, uint32_t instruction) {
    uint8_t opcode = instruction >> 26;

    if (opcode == 18) { //Branch
        bool AA = instruction & 2;
        bool LK = instruction & 1;

        uint32_t LI = instruction & 0x3FFFFFC;
        if (LI & 0x2000000) LI -= 0x4000000;

        if (LK) {
            state->context.lr = state->context.srr0 + 4;
        }

        if (AA) state->context.srr0 = LI;
        else {
            state->context.srr0 += LI;
        }
        return;
    }

    if (opcode == 16) { //Conditional branch
        bool AA = instruction & 2;

        uint32_t BD = instruction & 0xFFFC;
        if (BD & 0x8000) BD -= 0x10000;

        uint32_t target;
        if (AA) target = BD;
        else {
            target = state->context.srr0 + BD;
        }

        branchConditional(state, instruction, target, true);
        return;
    }

    if (opcode == 19) { //Conditional branch to lr/ctr
        uint16_t XO = (instruction >> 1) & 0x3FF;

        uint32_t target;
        if (XO == 16) {
            branchConditional(state, instruction, state->context.lr, true);
            return;
        } else if (XO == 528) {
            branchConditional(state, instruction, state->context.ctr, false);
            return;
        }
    }

    uint32_t *ptr = alloc();
    ptr[0]        = instruction;
    ptr[1]        = TRAP;
    ptr[2]        = state->context.srr0;
    DCFlushRange(ptr, 12);
    ICInvalidateRange(ptr, 12);
    state->context.srr0 = (uint32_t) ptr;
}

void StepMgr::handleBreakPoint(ExceptionState *state) {
    uint32_t start = (uint32_t) buffer;
    uint32_t end   = (uint32_t) buffer + sizeof(buffer);

    uint32_t addr = state->context.srr0;
    if (addr >= start && addr < end) {
        int offset = addr - start;
        if (offset % 12 != 4) {
            char buffer[100];
            snprintf(buffer, 100, "Unexpected srr0 after displaced step: %08X", addr);
            OSFatal(buffer);
        }

        int index           = offset / 12;
        state->context.srr0 = buffer[index * 3 + 2] + 4;
        free(index);

        state->resume();
    }
}

void StepMgr::adjustAddress(ExceptionState *state) {
    uint32_t start = (uint32_t) buffer;
    uint32_t end   = (uint32_t) buffer + sizeof(buffer);

    uint32_t addr = state->context.srr0;
    if (addr >= start && addr < end) {
        int index           = (addr - start) / 12;
        state->context.srr0 = buffer[index * 3 + 2];
    }
}


bool Debugger::checkDataRead(uint32_t addr, uint32_t length) {
    uint32_t memStart, memSize;
    OSGetMemBound(OS_MEM2, &memStart, &memSize);

    uint32_t memEnd = memStart + memSize;

    return addr >= 0x10000000 && addr + length <= memEnd;
}

Debugger::StepCommand Debugger::notifyBreak(ExceptionState *state) {
    OSMessage message;
    message.message = (void *) ExceptionState::PROGRAM;
    message.args[0] = (uint32_t) &state->context;
    message.args[1] = sizeof(OSContext);
    message.args[2] = (uint32_t) state->thread;
    OSSendMessage(&eventQueue, &message, OS_MESSAGE_FLAGS_BLOCKING);

    state->isPaused = true;
    OSReceiveMessage(&state->queue, &message, OS_MESSAGE_FLAGS_BLOCKING);
    state->isPaused = false;
    return (StepCommand) (uint32_t) message.message;
}

void Debugger::resumeBreakPoint(ExceptionState *state) {
    uint32_t address = state->context.srr0;
    if (breakpoints.isSoftware(address)) {
        state->context.srr0 += 4;
        state->resume();
    }

    uint32_t instruction = breakpoints.getInstr(state->context.srr0);
    stepper.singleStep(state, instruction);
    state->resume();
}

void Debugger::processBreakPoint(ExceptionState *state) {
    StepCommand command = notifyBreak(state);
    if (command == STEP_INTO || command == STEP_OVER) {
        breakpoints.predictStep(state, command == STEP_OVER);
    }
}

void Debugger::handleBreakPoint(ExceptionState *state) {
    if (firstTrap) {
        firstTrap = false;

        Screen screen;
        screen.init();
        Screen::drawText(
                0, 0, "Waiting for debugger connection.\n"
                      "Press the home button to continue without debugger.\n"
                      "You can still connect while the game is running.");
        Screen::flip();

        while (!connected) {
            uint32_t buttons = GetInput(VPAD_BUTTON_HOME);
            if (buttons) {
                DEBUG_FUNCTION_LINE("Pressed home");
                state->context.srr0 += 4;
                state->resume();
            }
        }
    }

    DEBUG_FUNCTION_LINE("stepper.handleBreakPoint(state);");

    stepper.handleBreakPoint(state);

    if (!connected) {
        DEBUG_FUNCTION_LINE("if (!connected) {");
        handleFatalCrash(&state->context, state->type);
    }

    uint32_t addr = state->context.srr0;
    bool trap;

    breakpoints.lock();
    trap = breakpoints.isCustom(addr) || breakpoints.isSoftware(addr) ||
           breakpoints.findSpecial(addr, state->thread);
    breakpoints.unlock();

    breakpoints.clearSpecial(state->thread);
    if (trap) {
        processBreakPoint(state);
    }
    resumeBreakPoint(state);
}

void Debugger::handleCrash(ExceptionState *state) {
    stepper.adjustAddress(state);
    if (connected) {
        OSMessage message;
        message.message = (void *) state->type;
        message.args[0] = (uint32_t) &state->context;
        message.args[1] = sizeof(OSContext);
        message.args[2] = (uint32_t) state->thread;
        OSSendMessage(&eventQueue, &message, OS_MESSAGE_FLAGS_BLOCKING);

        while (true) {
            OSReceiveMessage(&state->queue, &message, OS_MESSAGE_FLAGS_BLOCKING);
        }
    } else {
        handleFatalCrash(&state->context, state->type);
    }
}

void Debugger::handleFatalCrash(OSContext *context, ExceptionState::Type type) {
    const char *name;
    if (type == ExceptionState::DSI) name = "A DSI";
    else if (type == ExceptionState::ISI)
        name = "An ISI";
    else {
        name = "A program";
    }
    DumpContext(context, name);
}

void Debugger::handleException(OSContext *context, ExceptionState::Type type) {
    OSThread *thread = OSGetCurrentThread();

    if (thread == serverThread) {
        handleFatalCrash(context, type);
    }

    ExceptionState *state = exceptions.findOrCreate(thread);
    memcpy(&state->context, context, sizeof(OSContext));
    state->type = type;

    delete context;

    if (state->isBreakpoint()) {
        handleBreakPoint(state);
    } else {
        handleCrash(state);
    }
}

void Debugger::exceptionHandler(OSContext *context, ExceptionState::Type type) {
    debugger->handleException(context, type);
}

BOOL Debugger::dsiHandler(OSContext *context) {
    OSContext *info = new OSContext();
    memcpy(info, context, sizeof(OSContext));
    context->srr0   = (uint32_t) exceptionHandler;
    context->gpr[3] = (uint32_t) info;
    context->gpr[4] = ExceptionState::DSI;
    return true;
}

BOOL Debugger::isiHandler(OSContext *context) {
    OSContext *info = new OSContext();
    memcpy(info, context, sizeof(OSContext));
    context->srr0   = (uint32_t) exceptionHandler;
    context->gpr[3] = (uint32_t) info;
    context->gpr[4] = ExceptionState::ISI;
    return true;
}

BOOL Debugger::programHandler(OSContext *context) {
    OSContext *info = new OSContext();
    memcpy(info, context, sizeof(OSContext));
    context->srr0   = (uint32_t) exceptionHandler;
    context->gpr[3] = (uint32_t) info;
    context->gpr[4] = ExceptionState::PROGRAM;
    return true;
}

void Debugger::cleanup() {
    breakpoints.cleanup();
    exceptions.cleanup();

    OSMessage message;
    while (OSReceiveMessage(&eventQueue, &message, OS_MESSAGE_FLAGS_NONE))
        ;
}

extern "C" void OSRestoreInterrupts(int state);
extern "C" void __OSLockScheduler(void *);
extern "C" void __OSUnlockScheduler(void *);
extern "C" int OSDisableInterrupts();

static const char **commandNames = (const char *[]){
        "COMMAND_CLOSE",
        "COMMAND_READ",
        "COMMAND_WRITE",
        "COMMAND_WRITE_CODE",
        "COMMAND_GET_MODULE_NAME",
        "COMMAND_GET_MODULE_LIST",
        "COMMAND_GET_THREAD_LIST",
        "COMMAND_GET_STACK_TRACE",
        "COMMAND_TOGGLE_BREAKPOINT",
        "COMMAND_POKE_REGISTERS",
        "COMMAND_RECEIVE_MESSAGES",
        "COMMAND_SEND_MESSAGE"};

void Debugger::mainLoop(Client *client) {
    DEBUG_FUNCTION_LINE("About to enter mainLoop while");
    while (!stopRunning) {
        uint8_t cmd;
        if (!client->recvall(&cmd, 1)) return;

        if (cmd <= 11) {
            DEBUG_FUNCTION_LINE("Recieved command %s %d", commandNames[cmd], cmd);
        }
        if (cmd == COMMAND_CLOSE) {
            return;
        } else if (cmd == COMMAND_READ) {
            uint32_t addr, length;
            if (!client->recvall(&addr, 4)) return;
            if (!client->recvall(&length, 4)) return;

            char *buffer = new char[length];
            breakpoints.read(buffer, addr, length);
            if (!client->sendall(buffer, length)) {
                delete[] buffer;
                return;
            }
            delete[] buffer;
        } else if (cmd == COMMAND_WRITE) {
            uint32_t addr, length;
            if (!client->recvall(&addr, 4)) return;
            if (!client->recvall(&length, 4)) return;
            if (!client->recvall((void *) addr, length)) return;
        } else if (cmd == COMMAND_WRITE_CODE) {
            uint32_t addr, length;
            if (!client->recvall(&addr, 4)) return;
            if (!client->recvall(&length, 4)) return;

            char *buffer = new char[length];
            if (!client->recvall(buffer, length)) {
                delete[] buffer;
                return;
            }
            breakpoints.write(buffer, addr, length);
            delete[] buffer;
        } else if (cmd == COMMAND_GET_MODULE_NAME) {
            char name[0x40];
            int length = 0x40;
            OSDynLoad_GetModuleName(reinterpret_cast<OSDynLoad_Module>(-1), name, &length);

            length = strlen(name);
            if (!client->sendall(&length, 4)) return;
            if (!client->sendall(name, length)) return;
        } else if (cmd == COMMAND_GET_MODULE_LIST) {
            int num_rpls = OSDynLoad_GetNumberOfRPLs();
            if (num_rpls == 0) {
                return;
            }

            std::vector<OSDynLoad_NotifyData> rpls;
            rpls.resize(num_rpls);

            bool ret = OSDynLoad_GetRPLInfo(0, num_rpls, rpls.data());
            if (!ret) {
                return;
            }

            char buffer[0x1000]; //This should be enough
            uint32_t offset = 0;

            for (auto &info : rpls) {
                uint32_t namelen = strlen(info.name);
                if (offset + 0x18 + namelen > 0x1000) {
                    break;
                }
                auto *infobuf = (uint32_t *) (buffer + offset);
                infobuf[0]    = info.textAddr;
                infobuf[1]    = info.textSize;
                infobuf[2]    = info.dataAddr;
                infobuf[3]    = info.dataSize;
                infobuf[4]    = (uint32_t) 0; // TODO: missing
                infobuf[5]    = namelen;
                memcpy(&infobuf[6], info.name, namelen);
                offset += 0x18 + strlen(info.name);
            }

            //OSUnlockMutex(OSDynLoad_gLoaderLock);

            if (!client->sendall(&offset, 4)) return;
            if (!client->sendall(buffer, offset)) return;
        } else if (cmd == COMMAND_GET_THREAD_LIST) {
            int state = OSDisableInterrupts();
            __OSLockScheduler(this);

            char buffer[0x1000]; //This should be enough
            uint32_t offset   = 0;
            OSThread *current = ThreadList;
            while (current) {
                const char *name = OSGetThreadName(current);

                uint32_t namelen = 0;
                if (name) {
                    namelen = strlen(name);
                }

                if (offset + 0x1C + namelen > 0x1000) {
                    break;
                }

                int priority = current->basePriority;
                int type     = *(uint32_t *) (current->__unk11);
                if (type == 1) {
                    priority -= 0x20;
                } else if (type == 2) {
                    priority -= 0x40;
                }

                uint32_t *infobuf = (uint32_t *) (buffer + offset);
                infobuf[0]        = (uint32_t) current;
                infobuf[1]        = current->attr & 7;
                infobuf[2]        = priority;
                infobuf[3]        = (uint32_t) current->stackStart;
                infobuf[4]        = (uint32_t) current->stackEnd;
                infobuf[5]        = (uint32_t) current->entryPoint;
                infobuf[6]        = namelen;
                memcpy(&infobuf[7], name, namelen);
                offset += 0x1C + namelen;

                current = current->activeLink.next;
            }

            __OSUnlockScheduler(this);
            OSRestoreInterrupts(state);

            if (!client->sendall(&offset, 4)) return;
            if (!client->sendall(buffer, offset)) return;
        } else if (cmd == COMMAND_GET_STACK_TRACE) {
            OSThread *thread;
            if (!client->recvall(&thread, 4)) return;

            ExceptionState *state = exceptions.find(thread);
            if (state) {
                uint32_t sp = state->context.gpr[1];
                uint32_t trace[100];
                int index = 0;
                while (checkDataRead(sp, 4) && index < 100) {
                    sp = *(uint32_t *) sp;
                    if (!checkDataRead(sp, 4)) break;

                    trace[index] = *(uint32_t *) (sp + 4);
                    index++;
                }

                if (!client->sendall(&index, 4)) return;
                if (!client->sendall(trace, index * 4)) return;
            } else {
                int index = 0;
                if (!client->sendall(&index, 4)) return;
            }
        } else if (cmd == COMMAND_TOGGLE_BREAKPOINT) {
            uint32_t address;
            if (!client->recvall(&address, 4)) return;

            breakpoints.toggle(address);
        } else if (cmd == COMMAND_POKE_REGISTERS) {
            OSThread *thread;
            if (!client->recvall(&thread, 4)) return;

            uint32_t gpr[32];
            double fpr[32];
            if (!client->recvall(gpr, 4 * 32)) return;
            if (!client->recvall(fpr, 8 * 32)) return;

            exceptions.lock();
            ExceptionState *state = exceptions.find(thread);
            if (state) {
                memcpy(state->context.gpr, gpr, 4 * 32);
                memcpy(state->context.fpr, fpr, 8 * 32);
            }
            exceptions.unlock();
        } else if (cmd == COMMAND_RECEIVE_MESSAGES) {
            OSMessage messages[10];

            int count = 0;
            while (count < 10) {
                if (!OSReceiveMessage(&eventQueue, &messages[count], OS_MESSAGE_FLAGS_NONE)) {
                    break;
                }
                count++;
            }

            if (!client->sendall(&count, 4)) return;
            for (int i = 0; i < count; i++) {
                if (!client->sendall(&messages[i], sizeof(OSMessage))) return;
                if (messages[i].args[0]) {
                    void *data    = (void *) messages[i].args[0];
                    size_t length = messages[i].args[1];
                    if (!client->sendall(data, length)) return;
                }
            }
        } else if (cmd == COMMAND_SEND_MESSAGE) {
            OSMessage message;
            if (!client->recvall(&message, sizeof(OSMessage))) return;

            auto *thread = (OSThread *) message.args[0];

            exceptions.lock();
            ExceptionState *state = exceptions.find(thread);
            if (state) {
                OSSendMessage(&state->queue, &message, OS_MESSAGE_FLAGS_NONE);
            }
            exceptions.unlock();
        } else {
            DEBUG_FUNCTION_LINE("Recieved unknown command %d", cmd);
        }
    }
}

void Debugger::threadFunc() {
    DEBUG_FUNCTION_LINE("Hello from debugger thread :)!");
    OSSetExceptionCallbackEx(OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OS_EXCEPTION_TYPE_DSI, dsiHandler);
    OSSetExceptionCallbackEx(OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OS_EXCEPTION_TYPE_ISI, isiHandler);
    OSSetExceptionCallbackEx(OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OS_EXCEPTION_TYPE_PROGRAM, programHandler);
    DEBUG_FUNCTION_LINE("Callback init done.!");

    Server server;
    Client client;

    DEBUG_FUNCTION_LINE("Set initialized = true");
    initialized = true;
    while (!stopRunning) {
        if (!server.init(Socket::TCP)) continue;
        if (!server.bind(1560)) continue;
        if (!server.accept(&client)) continue;
        DEBUG_FUNCTION_LINE("Accepted a connection");
        connected = true;
        mainLoop(&client);
        DEBUG_FUNCTION_LINE("Lets do some cleanup");
        cleanup();
        connected = false;
        client.close();
        server.close();
    }
}

int Debugger::threadEntry(int argc, const char **argv) {
    DEBUG_FUNCTION_LINE("threadEntry");
    debugger->threadFunc();
    return 0;
}

void Debugger::start() {
    initialized = false;
    connected   = false;
    firstTrap   = true;

    DEBUG_FUNCTION_LINE("OSInitMessageQueue");
    OSInitMessageQueue(&eventQueue, eventMessages, MESSAGE_COUNT);

    DEBUG_FUNCTION_LINE("init breakpoints");
    breakpoints.init();
    DEBUG_FUNCTION_LINE("init exceptions");
    exceptions.init();
    DEBUG_FUNCTION_LINE("init stepper");
    stepper.init();


    DEBUG_FUNCTION_LINE("Alloc thread");
    serverThread = (OSThread *) memalign(0x20, sizeof(OSThread));
    DEBUG_FUNCTION_LINE("Alloc stack");
    serverStack = (char *) memalign(0x20, STACK_SIZE);


    DEBUG_FUNCTION_LINE("Create thread");
    OSCreateThread(
            serverThread, threadEntry, 0, 0,
            serverStack + STACK_SIZE, STACK_SIZE,
            0, 12);
    DEBUG_FUNCTION_LINE("Set thread name");
    OSSetThreadName(serverThread, "Debug Server");
    DEBUG_FUNCTION_LINE("Resume thread");
    OSResumeThread(serverThread);

    while (!initialized) {
        DEBUG_FUNCTION_LINE("Wait for thread init");
        OSSleepTicks(OSMillisecondsToTicks(20));
    }
    DEBUG_FUNCTION_LINE("Thread init done! Exit start()");
}

Debugger::~Debugger() {
    stopRunning = true;
    OSJoinThread(serverThread, nullptr);
    free(serverStack);
    serverStack = nullptr;
    free(serverThread);
    serverThread = nullptr;
}
