# MOS 6502 CPU Emulator & Interactive Debugger

A cycle-accurate MOS 6502 microprocessor emulator and desktop visual debugging suite written in **C++17**, powered by **Dear ImGui**, **GLFW**, and **OpenGL**.

The emulator features a verified CPU core that successfully passes the **Klaus Dormann 6502 Functional Test Suite** (~84 million cycles) alongside an extensive GoogleTest suite. It includes an interactive hardware debugger with real-time disassembly, register change tracking, an inline-editable 64KB memory viewer, a dedicated stack viewer, and configurable execution controls.

---

## Features

### 🖥️ Core Emulator (`CPU` & `Memory`)
- **Accurate Instruction Set**: Comprehensive implementation of official MOS 6502 instructions and addressing modes (Immediate, Zero Page, Zero Page X/Y, Absolute, Absolute X/Y, Indirect X/Y, Relative, Implied/Accumulator).
- **Exact Cycle Accounting**: Single-instruction stepping (`Step()`) and cycle-budgeted batch execution (`Execute()`) track hardware clock cycles.
- **Hardware Vectors**: Supports standard 6502 vectors including Reset (`$FFFC/$FFFD`), IRQ/BRK (`$FFFE/$FFFF`), and NMI.
- **Thoroughly Verified**: Fully passes Klaus Dormann's functional test suite and 17 dedicated GoogleTest unit test targets.

### 🛠️ Interactive GUI Debugger
- **Modern Dark Theme**: Professional charcoal aesthetic with high-contrast color-coded indicators (green flags/active states, red breakpoints, blue selections, gold change highlights).
- **CPU Registers Panel**: Displays Accumulator (`A`), `X`, `Y`, Program Counter (`PC`), Stack Pointer (`SP`), and Processor Status (`P`) in both hexadecimal and decimal, with real-time visual change highlighting when registers update.
- **Processor Status Flags**: Individual color-coded status badges for **N** (Negative), **V** (Overflow), **B** (Break), **D** (Decimal), **I** (Interrupt Disable), **Z** (Zero), and **C** (Carry).
- **Surrounding Disassembly Viewer**: Real-time 256-opcode disassembler with operand resolution, addressing modes, current PC highlight (`->`), auto-scroll following execution, and single-click breakpoint toggling.
- **64KB Memory Hex Editor**: Virtual-scrolled hex view with 16 bytes per row, ASCII translation column, jump-to-address navigation, PC highlight, and **live inline memory editing**.
- **Dedicated Stack Viewer**: Focused inspection of the 6502 hardware stack (`$0100`–`$01FF`) with stack pointer marker (`<-- SP / TOS`).
- **Execution Controls**:
  - **Run (F5)**: Continuous execution at user-configurable clock frequencies (1 Hz up to uncapped).
  - **Pause**: Halts continuous execution immediately.
  - **Step (F10)**: Executes exactly one complete 6502 instruction.
  - **Step Over (F11)**: Steps over subroutines (`JSR`) by running until return.
  - **Reset**: Reloads the PC from the reset vector (`$FFFC/$FFFD`) and initializes CPU state.
  - **Step N Instructions / Run N Cycles**: Precise numeric execution controls.
- **Breakpoints**: Add, remove, and manage multiple address breakpoints. Execution pauses immediately prior to executing an instruction at any active breakpoint.
- **Binary Loader**: Load custom binary payloads into any memory location directly from the GUI.

---

## Technology Stack

- **Language**: C++17
- **GUI Framework**: [Dear ImGui](https://github.com/ocornut/imgui) (v1.91.5)
- **Window Management**: [GLFW](https://www.glfw.org/) (3.4)
- **Graphics Backend**: OpenGL 3.3 Core (GLSL 130)
- **Build System**: CMake (v3.14+)
- **Testing**: [GoogleTest](https://github.com/google/googletest) (release-1.12.1)
- **Assembler**: TMPx (for compiling custom 6502 test programs)

---

## Project Structure

```
6502 Emulator/
├── m6502.h / m6502.cpp          # Core MOS 6502 CPU and Memory implementation
├── main.cpp                     # CLI application entry point
├── CMakeLists.txt              # CMake build configuration
│
├── debugger/                    # Desktop Debugger GUI (ImGui + GLFW + OpenGL)
│   ├── main_gui.cpp            # Window initialization & main application loop
│   ├── DebuggerUI.h / .cpp     # Dark theme styling and ImGui panel layout
│   ├── Debugger.h / .cpp       # Debugger state machine, breakpoints, & stepping
│   └── Disassembler.h / .cpp   # 256-opcode static decoder & instruction formatter
│
├── testing/                     # GoogleTest test suites
│   ├── test_load.cpp           # LDA / LDX / LDY tests
│   ├── test_store.cpp          # STA / STX / STY tests
│   ├── test_addcarry.cpp       # ADC tests (signed overflow, carry, decimal)
│   ├── test_subtractcarry.cpp  # SBC tests
│   ├── test_branch.cpp         # Branching instructions (BEQ, BNE, BCS, BCC, etc.)
│   ├── test_jump.cpp           # JMP, JSR, RTS tests
│   ├── test_stack.cpp          # Stack operations (PHA, PLA, PHP, PLP, TSX, TXS)
│   ├── test_shift_left.cpp     # ASL / ROL tests
│   ├── test_shift_right.cpp    # LSR / ROR tests
│   ├── test_logical.cpp        # AND / ORA / EOR / BIT tests
│   └── ...                     # Additional opcode suites
│
├── klaus test/                  # Klaus Dormann 6502 Functional Verification
│   ├── 6502_functional_test.bin
│   └── klaustest.cpp           # Test harness driving ~84M test cycles
│
└── compile/                     # Custom 6502 Assembly compiler tooling
    ├── TMPx.exe                # TMPx 6502 cross-assembler
    ├── program.s               # Sample assembly program
    └── program.bin             # Assembled binary output
```

---

## Getting Started

### Prerequisites

- A C++17 compatible compiler:
  - **Windows**: MinGW-w64 (`g++` 10+) or MSVC (Visual Studio 2019+)
  - **Linux**: `g++` 10+ or `clang++` 11+
  - **macOS**: Apple Clang
- **CMake** 3.14 or newer
- **Git** (for CMake `FetchContent` to download dependencies automatically)
- OpenGL development libraries (included by default on Windows/macOS; on Linux: `libgl1-mesa-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev`)

---

### Building the Project

1. **Clone the repository:**
   ```bash
   git clone https://github.com/JosephJoshy2007/6502-Emulator.git
   cd 6502-Emulator
   ```

2. **Configure CMake:**
   - On Windows (MinGW):
     ```powershell
     cmake -B build -G "MinGW Makefiles"
     ```
   - On Windows (MSVC) or Linux/macOS:
     ```bash
     cmake -B build
     ```

3. **Build the targets:**
   ```bash
   # Build the GUI Debugger
   cmake --build build --target 6502_debugger

   # Build all targets (Debugger, CLI app, and Unit Tests)
   cmake --build build
   ```

---

### Running the GUI Debugger

Once compiled, launch the interactive debugger:

- **Windows:**
  ```powershell
  .\build\6502_debugger.exe
  ```
- **Linux / macOS:**
  ```bash
  ./build/6502_debugger
  ```

By default, the debugger attempts to automatically load `compile/program.bin` at `$8000` and configures the reset vector. You can also load any custom binary using the input field on the top toolbar.

#### Keyboard Shortcuts in the Debugger
| Key | Action |
|---|---|
| **F5** | Run / Resume execution |
| **F10** | Step (single instruction) |
| **F11** | Step Over (subroutines) |

---

### Running Tests

#### 1. GoogleTest Unit Tests
Run the individual test executables generated inside `build/`:
```powershell
# Example: Run branch tests
.\build\6502_branch_tests.exe

# Example: Run arithmetic tests
.\build\6502_addcarry_tests.exe
```

#### 2. Klaus Dormann Functional Test
Compile and run the Klaus Dormann functional test harness:
```powershell
# Compile the test runner
g++ -std=c++17 -O3 "klaus test/klaustest.cpp" m6502.cpp -I. -o "klaus test/klaustest.exe"

# Execute the test
& "klaus test/klaustest.exe"
```
The test verifies instruction behavior over ~84 million cycles and successfully terminates at the trap address (`$336D`).

---

### Writing & Assembling Custom 6502 Programs

You can write custom 6502 assembly in `compile/program.s` and assemble it using the included **TMPx** assembler:

> [!NOTE]
> TMPx labels must **not** include trailing colons (e.g., use `start` rather than `start:`).

```assembly
*= $8000

start
    LDA #$0D        ; Multiplicand (13)
    STA $10
    LDA #$11        ; Multiplier (17)
    STA $11
    LDA #$00        ; Result
    STA $12
    LDY $11

multiply
    CLC
    LDA $12
    ADC $10
    STA $12
    DEY
    BNE multiply

    LDA #$01
    STA $1F         ; Mark complete

halt
    JMP halt
```

Assemble using TMPx:
```powershell
cd compile
.\TMPx.exe -i program.s -o program.bin
```

---

## Contributing

Contributions, bug reports, and optimizations are welcome!
1. Fork the repository.
2. Create your feature branch (`git checkout -b feature/MyFeature`).
3. Commit your changes (`git commit -m "Add some feature"`).
4. Push to the branch (`git push origin feature/MyFeature`).
5. Open a Pull Request.

---

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
