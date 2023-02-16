#pragma once

#include <coreinit/thread.h>
#include <cstdint>

#define IDLE_THREAD_ID_CORE_0                 0x7001
#define IDLE_THREAD_ID_CORE_1                 0x7002
#define IDLE_THREAD_ID_CORE_2                 0x7003

#define IDLE_THREAD_CORE_0                    ((OSThread *) 0x100)
#define IDLE_THREAD_CORE_1                    ((OSThread *) 0x200)
#define IDLE_THREAD_CORE_2                    ((OSThread *) 0x300)
#define IDLE_THREAD_LIST_END                  ((OSThread *) 0x400)
#define END_OF_LIST_THREAD                    ((OSThread *) 0xFFFFFFFF)

#define GET_CORE_FROM_IDLE_THREAD(thread)     ((uint32_t) thread >> 8 & 3) - 1;

#define DEFAULT_THREAD_ID                     0x8000

#define IS_IDLE_THREAD(thread)                (((uint32_t) thread & ~0x300) == 0)
#define GET_NEXT_IDLE_THREAD(thread)          (OSThread *) (((uint32_t) thread & 0x300) + 0x100)
#define GET_IDLE_THREAD_BY_CORE(coreOfThread) (OSThread *) ((coreOfThread * 0x100) + 0x100);
