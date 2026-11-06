#pragma once

#include "../m6502.h"
#include <set>
#include <string>
#include <cstdint>

enum class DebuggerState {
    Paused,
    Running,
    StepOverActive
};

class Debugger {
public:
    CPU& cpu;
    Memory& memory;

    DebuggerState state = DebuggerState::Paused;

    // Statistics
    uint64_t totalInstructions = 0;
    uint64_t totalCycles = 0;
    Byte lastOpcode = 0;
    Word lastPC = 0;

    // Previous CPU state for change highlighting
    Byte prevA = 0;
    Byte prevX = 0;
    Byte prevY = 0;
    Byte prevSP = 0;
    Word prevPC = 0;
    Byte prevP = 0;

    // Breakpoints
    std::set<Word> breakpoints;
    bool hitBreakpoint = false;
    Word breakpointHitAddress = 0;
    Word stepOverTargetPC = 0;

    // Execution configuration
    int instructionsPerFrame = 500; // Configurable speed

    // Program status
    std::string loadedFilePath = "";
    bool hasLoadedProgram = false;

    Debugger(CPU& cpu, Memory& memory);

    void Reset();
    void Step();
    void StepOver();
    void Run();
    void Pause();

    void ToggleBreakpoint(Word address);
    void AddBreakpoint(Word address);
    void RemoveBreakpoint(Word address);
    void ClearBreakpoints();
    bool HasBreakpoint(Word address) const;

    void ExecuteCycles(int budget);
    void ExecuteInstructions(int count);

    // Call once per UI frame to advance execution when running
    void Update(float deltaTime);

    bool LoadBinary(const std::string& filepath, Word customLoadAddr = 0);

private:
    void SavePreviousState();
};
