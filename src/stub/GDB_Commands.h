#pragma once
#include <string_view>

static constexpr std::string_view GDBTargetXML = R"(<?xml version="1.0"?>
<!DOCTYPE target SYSTEM "gdb-target.dtd">
<target version="1.0">
    <architecture>powerpc:common</architecture>
    <feature name="org.gnu.gdb.power.core">
        <reg name="r0" bitsize="32" type="uint32"/>
        <reg name="r1" bitsize="32" type="uint32"/>
        <reg name="r2" bitsize="32" type="uint32"/>
        <reg name="r3" bitsize="32" type="uint32"/>
        <reg name="r4" bitsize="32" type="uint32"/>
        <reg name="r5" bitsize="32" type="uint32"/>
        <reg name="r6" bitsize="32" type="uint32"/>
        <reg name="r7" bitsize="32" type="uint32"/>
        <reg name="r8" bitsize="32" type="uint32"/>
        <reg name="r9" bitsize="32" type="uint32"/>
        <reg name="r10" bitsize="32" type="uint32"/>
        <reg name="r11" bitsize="32" type="uint32"/>
        <reg name="r12" bitsize="32" type="uint32"/>
        <reg name="r13" bitsize="32" type="uint32"/>
        <reg name="r14" bitsize="32" type="uint32"/>
        <reg name="r15" bitsize="32" type="uint32"/>
        <reg name="r16" bitsize="32" type="uint32"/>
        <reg name="r17" bitsize="32" type="uint32"/>
        <reg name="r18" bitsize="32" type="uint32"/>
        <reg name="r19" bitsize="32" type="uint32"/>
        <reg name="r20" bitsize="32" type="uint32"/>
        <reg name="r21" bitsize="32" type="uint32"/>
        <reg name="r22" bitsize="32" type="uint32"/>
        <reg name="r23" bitsize="32" type="uint32"/>
        <reg name="r24" bitsize="32" type="uint32"/>
        <reg name="r25" bitsize="32" type="uint32"/>
        <reg name="r26" bitsize="32" type="uint32"/>
        <reg name="r27" bitsize="32" type="uint32"/>
        <reg name="r28" bitsize="32" type="uint32"/>
        <reg name="r29" bitsize="32" type="uint32"/>
        <reg name="r30" bitsize="32" type="uint32"/>
        <reg name="r31" bitsize="32" type="uint32"/>
        <reg name="pc" bitsize="32" type="code_ptr" regnum="64"/>
        <reg name="msr" bitsize="32" type="uint32"/>
        <reg name="cr" bitsize="32" type="uint32"/>
        <reg name="lr" bitsize="32" type="code_ptr"/>
        <reg name="ctr" bitsize="32" type="uint32"/>
        <reg name="xer" bitsize="32" type="uint32"/>
    </feature>
    <feature name="org.gnu.gdb.power.fpu">
        <reg name="f0" bitsize="64" type="ieee_double" regnum="32"/>
        <reg name="f1" bitsize="64" type="ieee_double"/>
        <reg name="f2" bitsize="64" type="ieee_double"/>
        <reg name="f3" bitsize="64" type="ieee_double"/>
        <reg name="f4" bitsize="64" type="ieee_double"/>
        <reg name="f5" bitsize="64" type="ieee_double"/>
        <reg name="f6" bitsize="64" type="ieee_double"/>
        <reg name="f7" bitsize="64" type="ieee_double"/>
        <reg name="f8" bitsize="64" type="ieee_double"/>
        <reg name="f9" bitsize="64" type="ieee_double"/>
        <reg name="f10" bitsize="64" type="ieee_double"/>
        <reg name="f11" bitsize="64" type="ieee_double"/>
        <reg name="f12" bitsize="64" type="ieee_double"/>
        <reg name="f13" bitsize="64" type="ieee_double"/>
        <reg name="f14" bitsize="64" type="ieee_double"/>
        <reg name="f15" bitsize="64" type="ieee_double"/>
        <reg name="f16" bitsize="64" type="ieee_double"/>
        <reg name="f17" bitsize="64" type="ieee_double"/>
        <reg name="f18" bitsize="64" type="ieee_double"/>
        <reg name="f19" bitsize="64" type="ieee_double"/>
        <reg name="f20" bitsize="64" type="ieee_double"/>
        <reg name="f21" bitsize="64" type="ieee_double"/>
        <reg name="f22" bitsize="64" type="ieee_double"/>
        <reg name="f23" bitsize="64" type="ieee_double"/>
        <reg name="f24" bitsize="64" type="ieee_double"/>
        <reg name="f25" bitsize="64" type="ieee_double"/>
        <reg name="f26" bitsize="64" type="ieee_double"/>
        <reg name="f27" bitsize="64" type="ieee_double"/>
        <reg name="f28" bitsize="64" type="ieee_double"/>
        <reg name="f29" bitsize="64" type="ieee_double"/>
        <reg name="f30" bitsize="64" type="ieee_double"/>
        <reg name="f31" bitsize="64" type="ieee_double"/>
        <reg name="fpscr" bitsize="32" group="float"  regnum="70"/>
    </feature>
</target>)";

void qSupported(const char *packet);
void qXferThreadRead(const char *packet);
bool qXferLibrariesRead(const char *packet);
bool qXferFeaturesRead(const char *packet);
void qOffsets(const char *packet);

#define gdb_continue            ((void (*)(const char *packet))(0x101C400 + 0x0203d11c - 0x02000000))
#define gdb_vgetregisters       ((void (*)(const char *packet))(0x101C400 + 0x0203dfb4 - 0x02000000))
#define gdb_get_pid_tid         ((OSThread * (*) (char **input))(0x101C400 + 0x0203d26c - 0x02000000))
#define GDBqXfer_libraries_read ((uint32_t(*)(uint32_t, int32_t, char *))(0x101C400 + 0x0203d3fc - 0x02000000))