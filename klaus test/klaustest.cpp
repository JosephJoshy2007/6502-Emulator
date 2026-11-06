#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <iomanip>
#include <cstdio>
#include <cstdint>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#if __has_include("../m6502.h")
#include "../m6502.h"
#else
#include "m6502.h"
#endif

namespace fs = std::filesystem;

// Helper to temporarily silence stdout (to suppress per-cycle / batch PrintState logs)
class StdoutSilencer {
    int saved_stdout = -1;
    bool active = false;
public:
    void silence() {
        if (active) return;
        std::fflush(stdout);
#ifdef _WIN32
        saved_stdout = _dup(_fileno(stdout));
        FILE* nul = std::fopen("NUL", "w");
        if (nul) {
            _dup2(_fileno(nul), _fileno(stdout));
            std::fclose(nul);
            active = true;
        }
#else
        saved_stdout = dup(fileno(stdout));
        FILE* nul = fopen("/dev/null", "w");
        if (nul) {
            dup2(fileno(nul), fileno(stdout));
            fclose(nul);
            active = true;
        }
#endif
    }

    void restore() {
        if (!active) return;
        std::fflush(stdout);
#ifdef _WIN32
        _dup2(saved_stdout, _fileno(stdout));
        _close(saved_stdout);
#else
        dup2(saved_stdout, fileno(stdout));
        close(saved_stdout);
#endif
        active = false;
    }

    ~StdoutSilencer() {
        restore();
    }
};

fs::path FindTestBinary() {
    std::vector<fs::path> candidates = {
        "6502_functional_test.bin",
        "klaus test/6502_functional_test.bin",
        "../klaus test/6502_functional_test.bin"
    };

    for (const auto& path : candidates) {
        if (fs::exists(path)) {
            return fs::absolute(path);
        }
    }

    fs::path current = fs::current_path();
    for (int i = 0; i < 4; ++i) {
        fs::path p1 = current / "6502_functional_test.bin";
        if (fs::exists(p1)) return p1;
        fs::path p2 = current / "klaus test" / "6502_functional_test.bin";
        if (fs::exists(p2)) return p2;
        if (!current.has_parent_path()) break;
        current = current.parent_path();
    }
    return {};
}

bool LoadKlausBinary(const fs::path& binPath, Memory& mem) {
    std::ifstream file(binPath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open binary file: " << binPath << "\n";
        return false;
    }

    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    if (fileSize <= 0) {
        std::cerr << "Error: Binary file is empty.\n";
        return false;
    }

    std::vector<uint8_t> buffer(fileSize);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), fileSize)) {
        std::cerr << "Error: Failed to read binary data from file.\n";
        return false;
    }

    // Klaus functional test binary starts at address 0x000A up to 0xFFFF
    constexpr uint32_t LOAD_START = 0x000A;
    for (size_t i = 0; i < buffer.size(); ++i) {
        uint32_t targetAddr = LOAD_START + static_cast<uint32_t>(i);
        if (targetAddr > 0xFFFF) {
            std::cerr << "Warning: Binary data exceeds 64KB boundary.\n";
            break;
        }
        mem[targetAddr] = buffer[i];
    }

    std::cout << "Successfully loaded " << buffer.size() << " bytes into memory from 0x" 
              << std::hex << std::uppercase << LOAD_START << " to 0x" 
              << (LOAD_START + buffer.size() - 1) << std::dec << "\n";
    return true;
}

int main(int argc, char* argv[]) {
    bool verbose = false;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        }
    }

    std::cout << "=========================================================\n";
    std::cout << "       Klaus Dormann 6502 Functional Test Runner         \n";
    std::cout << "=========================================================\n";

    fs::path binPath = FindTestBinary();
    if (binPath.empty()) {
        std::cerr << "Error: Could not find '6502_functional_test.bin'!\n";
        std::cerr << "Please make sure it exists in the 'klaus test' folder.\n";
        return 1;
    }

    std::cout << "Binary Path: " << binPath.string() << "\n";

    CPU cpu;
    Memory mem;

    // Reset CPU and Memory first (Reset calls mem.Initialize() which zeroes Data)
    cpu.Reset(mem);

    // Load the test binary from 0x000A to 0xFFFF
    if (!LoadKlausBinary(binPath, mem)) {
        return 1;
    }

    // Klaus Dormann functional test begins at address 0x0400 (start: CLD)
    constexpr Word KLAUS_START_PC = 0x0400;
    // 0x3469 = success when decimal mode tests are enabled
    // 0x336D = success when decimal mode tests are disabled
    auto IsSuccessPC = [](Word pc) {
        return (pc == 0x3469 || pc == 0x336D);
    };
    cpu.PC = KLAUS_START_PC;

    std::cout << "Entry Point (PC): 0x" << std::hex << std::uppercase 
              << cpu.PC << std::dec << "\n";
    if (!verbose) {
        std::cout << "Per-batch logging is silenced for maximum execution speed.\n";
        std::cout << "(Pass '-v' or '--verbose' to print every batch).\n";
    }
    std::cout << "Running test (up to 100M cycles)...\n\n";

    constexpr uint64_t MAX_CYCLES = 100000000ULL; // 100 Million cycles
    constexpr s32 BATCH_CYCLES = 5000;             // Execute in batches for high speed
    uint64_t totalCycles = 0;

    Word prevPC = 0xFFFF;
    int samePCCount = 0;
    bool trapped = false;
    bool success = false;

    StdoutSilencer silencer;
    if (!verbose) {
        silencer.silence();
    }

    while (totalCycles < MAX_CYCLES) {
        Word currentPC = cpu.PC;

        // Check if PC is looping at the exact same instruction
        if (currentPC == prevPC) {
            samePCCount++;
            if (samePCCount >= 2) {
                if (IsSuccessPC(currentPC)) {
                    success = true;
                } else {
                    trapped = true;
                }
                break;
            }
        } else {
            samePCCount = 0;
        }
        prevPC = currentPC;

        // Run batch
        s32 cyclesUsed = cpu.Execute(mem, BATCH_CYCLES);
        totalCycles += (cyclesUsed > 0 ? cyclesUsed : BATCH_CYCLES);

        // Check if trapped during this batch (e.g., self jump)
        if (cpu.PC == prevPC) {
            // Verify if it's a tight self-loop by stepping 1 instruction
            Word testPC = cpu.PC;
            cpu.Execute(mem, 1);
            if (cpu.PC == testPC) {
                if (IsSuccessPC(cpu.PC)) {
                    success = true;
                } else {
                    trapped = true;
                }
                break;
            }
        }
    }

    // Restore stdout for the diagnostic report
    silencer.restore();

    std::cout << "\n=========================================================\n";
    if (success) {
        std::cout << "  >>> KLAUS 6502 FUNCTIONAL TEST PASSED! <<<           \n";
        std::cout << "  Success loop reached at PC: 0x" << std::hex << std::uppercase 
                  << cpu.PC << std::dec << "\n";
    } else if (trapped) {
        std::cout << "  >>> TEST FAILED: TRAP DETECTED! <<<                  \n";
        std::cout << "  The emulator is stuck in an infinite loop at:\n";
        std::cout << "  PC: 0x" << std::hex << std::uppercase << std::setw(4) 
                  << std::setfill('0') << cpu.PC << std::dec << "\n";
    } else {
        std::cout << "  >>> TIMEOUT: Maximum cycle limit reached! <<<        \n";
        std::cout << "  Last PC: 0x" << std::hex << std::uppercase << std::setw(4) 
                  << std::setfill('0') << cpu.PC << std::dec << "\n";
    }
    std::cout << "=========================================================\n\n";

    // Summary statistics
    std::cout << "Execution Summary:\n";
    std::cout << "  Clock Cycles Consumed : ~" << totalCycles << " / " << MAX_CYCLES << "\n\n";

    // Final CPU state
    Byte status = cpu.GetStatusRegister();
    std::cout << "Final CPU State:\n";
    std::cout << "  PC: 0x" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << cpu.PC
              << "   SP: 0x" << std::setw(2) << static_cast<int>(cpu.SP)
              << "   A: 0x" << std::setw(2) << static_cast<int>(cpu.A)
              << "   X: 0x" << std::setw(2) << static_cast<int>(cpu.X)
              << "   Y: 0x" << std::setw(2) << static_cast<int>(cpu.Y) << "\n";
    std::cout << "  Status (P): 0x" << std::setw(2) << static_cast<int>(status)
              << " [NV-BDIZC]: "
              << (cpu.N ? 'N' : '.')
              << (cpu.V ? 'V' : '.')
              << '-'
              << ((status & 0x10) ? 'B' : '.')
              << (cpu.D ? 'D' : '.')
              << (cpu.I ? 'I' : '.')
              << (cpu.Z ? 'Z' : '.')
              << (cpu.C ? 'C' : '.')
              << std::dec << "\n";

    // Disassembly preview at the trapped PC
    std::cout << "\nMemory bytes at trapped PC (0x" << std::hex << std::uppercase << cpu.PC << "): ";
    for (int i = 0; i < 4; ++i) {
        Word addr = cpu.PC + i;
        std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(mem[addr]) << " ";
    }
    std::cout << std::dec << "\n";

    if (trapped) {
        std::cout << "\n---------------------------------------------------------\n";
        std::cout << "How to inspect the failure in VS Code:\n";
        std::cout << " 1. Open '6502_functional_test.lst' in VS Code.\n";
        std::cout << " 2. Press Ctrl + F and search for: " 
                  << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << cpu.PC << "\n";
        std::cout << " 3. Read the code and comments right above that line\n";
        std::cout << "    to see the exact test and assertion that failed!\n";
        std::cout << "---------------------------------------------------------\n";
    }

    return (success ? 0 : 1);
}
