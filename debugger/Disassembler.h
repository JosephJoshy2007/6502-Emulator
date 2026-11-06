#pragma once

#include "../m6502.h"
#include <string>
#include <vector>

enum class AddressingMode {
    Implied,
    Accumulator,
    Immediate,
    ZeroPage,
    ZeroPageX,
    ZeroPageY,
    Absolute,
    AbsoluteX,
    AbsoluteY,
    Indirect,
    IndirectX,
    IndirectY,
    Relative,
    Unknown
};

struct OpcodeInfo {
    const char* mnemonic;
    AddressingMode mode;
    int length; // 1, 2, or 3
};

struct DisassembledInstruction {
    Word address = 0;
    Byte opcode = 0;
    int length = 1;
    std::string bytesHex;     // e.g. "A9 42"
    std::string mnemonic;     // e.g. "LDA"
    std::string operand;      // e.g. "#$42"
    std::string fullText;     // e.g. "LDA #$42"
    AddressingMode mode = AddressingMode::Unknown;
};

class Disassembler {
public:
    static const OpcodeInfo& GetOpcodeInfo(Byte opcode);
    static const char* GetAddressingModeName(AddressingMode mode);
    
    // Disassemble a single instruction at address
    static DisassembledInstruction Disassemble(const Memory& mem, Word address);

    // Disassemble a range starting at startAddress, returning 'count' instructions
    static std::vector<DisassembledInstruction> DisassembleRange(const Memory& mem, Word startAddress, int count);

    // Disassemble around a target PC (before and after)
    static std::vector<DisassembledInstruction> DisassembleAround(const Memory& mem, Word targetPC, int linesBefore, int linesAfter);
};
