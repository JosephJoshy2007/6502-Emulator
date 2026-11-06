#pragma once

#include "Debugger.h"
#include <string>

class DebuggerUI {
public:
    static void SetupTheme();

    static void Render(Debugger& dbg);

private:
    static void RenderTopBar(Debugger& dbg);
    static void RenderRegistersAndFlags(Debugger& dbg);
    static void RenderExecutionControls(Debugger& dbg);
    static void RenderDisassembly(Debugger& dbg);
    static void RenderMemoryViewer(Debugger& dbg);
    static void RenderStackViewer(Debugger& dbg);
    static void RenderBreakpointsAndStats(Debugger& dbg);
};
