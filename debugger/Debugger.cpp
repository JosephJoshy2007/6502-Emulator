#include "Debugger.h"
#include <fstream>
#include <vector>
#include <iostream>

Debugger::Debugger(CPU& cpuRef, Memory& memoryRef)
    : cpu(cpuRef), memory(memoryRef) {
    SavePreviousState();
}

void Debugger::SavePreviousState() {
    prevA = cpu.A;
    prevX = cpu.X;
    prevY = cpu.Y;
    prevSP = cpu.SP;
    prevPC = cpu.PC;
    prevP = cpu.GetStatusRegister();
}

void Debugger::Reset() {
    cpu.Reset(memory);
    // Read 6502 reset vector at 0xFFFC/0xFFFD
    Word resetVector = static_cast<Word>(memory[0xFFFC]) | (static_cast<Word>(memory[0xFFFD]) << 8);
    cpu.PC = resetVector;
    
    state = DebuggerState::Paused;
    hitBreakpoint = false;
    breakpointHitAddress = 0;
    stepOverTargetPC = 0;
    totalInstructions = 0;
    totalCycles = 0;
    lastOpcode = memory[cpu.PC];
    lastPC = cpu.PC;

    SavePreviousState();
}

void Debugger::Step() {
    hitBreakpoint = false;
    SavePreviousState();

    lastPC = cpu.PC;
    lastOpcode = memory[cpu.PC];

    s32 cycles = cpu.Step(memory);
    totalCycles += (cycles > 0 ? cycles : 1);
    totalInstructions++;

    if (state == DebuggerState::StepOverActive) {
        if (cpu.PC == stepOverTargetPC) {
            state = DebuggerState::Paused;
        }
    }
}

void Debugger::StepOver() {
    Byte op = memory[cpu.PC];
    if (op == 0x20) { // JSR instruction length is 3 bytes
        stepOverTargetPC = static_cast<Word>(cpu.PC + 3);
        state = DebuggerState::StepOverActive;
    } else {
        Step();
    }
}

void Debugger::Run() {
    state = DebuggerState::Running;
    hitBreakpoint = false;
}

void Debugger::Pause() {
    state = DebuggerState::Paused;
    stepOverTargetPC = 0;
}

void Debugger::ToggleBreakpoint(Word address) {
    if (breakpoints.count(address)) {
        breakpoints.erase(address);
    } else {
        breakpoints.insert(address);
    }
}

void Debugger::AddBreakpoint(Word address) {
    breakpoints.insert(address);
}

void Debugger::RemoveBreakpoint(Word address) {
    breakpoints.erase(address);
}

void Debugger::ClearBreakpoints() {
    breakpoints.clear();
}

bool Debugger::HasBreakpoint(Word address) const {
    return breakpoints.count(address) > 0;
}

void Debugger::ExecuteCycles(int budget) {
    int consumed = 0;
    while (consumed < budget && state != DebuggerState::Paused) {
        if (breakpoints.count(cpu.PC)) {
            hitBreakpoint = true;
            breakpointHitAddress = cpu.PC;
            state = DebuggerState::Paused;
            break;
        }
        SavePreviousState();
        lastPC = cpu.PC;
        lastOpcode = memory[cpu.PC];
        s32 c = cpu.Step(memory);
        totalCycles += c;
        totalInstructions++;
        consumed += c;
    }
}

void Debugger::ExecuteInstructions(int count) {
    for (int i = 0; i < count; ++i) {
        if (breakpoints.count(cpu.PC)) {
            hitBreakpoint = true;
            breakpointHitAddress = cpu.PC;
            state = DebuggerState::Paused;
            break;
        }
        Step();
    }
}

void Debugger::Update(float /*deltaTime*/) {
    if (state == DebuggerState::Paused) {
        return;
    }

    if (state == DebuggerState::Running) {
        for (int i = 0; i < instructionsPerFrame; ++i) {
            // Check breakpoint before executing instruction
            if (breakpoints.count(cpu.PC)) {
                hitBreakpoint = true;
                breakpointHitAddress = cpu.PC;
                state = DebuggerState::Paused;
                break;
            }

            Step();
        }
    } else if (state == DebuggerState::StepOverActive) {
        // Run until target PC is reached, or a breakpoint is hit, or bound reached
        for (int i = 0; i < instructionsPerFrame; ++i) {
            if (breakpoints.count(cpu.PC)) {
                hitBreakpoint = true;
                breakpointHitAddress = cpu.PC;
                state = DebuggerState::Paused;
                break;
            }

            Step();

            if (cpu.PC == stepOverTargetPC) {
                state = DebuggerState::Paused;
                break;
            }
        }
    }
}

bool Debugger::LoadBinary(const std::string& filepath, Word customLoadAddr) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Debugger: Could not open binary file: " << filepath << "\n";
        return false;
    }

    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    if (fileSize < 2) {
        std::cerr << "Debugger: File is too small.\n";
        return false;
    }

    std::vector<uint8_t> buffer(fileSize);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), fileSize)) {
        return false;
    }

    // Check if user specified a custom address or if we should use PRG 2-byte header
    Word startAddress = 0;
    size_t dataOffset = 0;

    if (customLoadAddr != 0) {
        startAddress = customLoadAddr;
        dataOffset = 0;
    } else {
        // Standard Commodore PRG 2-byte little endian header
        startAddress = static_cast<Word>(buffer[0]) | (static_cast<Word>(buffer[1]) << 8);
        dataOffset = 2;
    }

    // Set reset vector so Reset() points to the loaded program
    memory[0xFFFC] = startAddress & 0xFF;
    memory[0xFFFD] = (startAddress >> 8) & 0xFF;

    for (size_t i = dataOffset; i < buffer.size(); ++i) {
        uint32_t currentAddress = startAddress + (i - dataOffset);
        if (currentAddress > 0xFFFF) break;
        memory[currentAddress] = buffer[i];
    }

    cpu.PC = startAddress;
    loadedFilePath = filepath;
    hasLoadedProgram = true;
    state = DebuggerState::Paused;
    hitBreakpoint = false;

    SavePreviousState();
    return true;
}
