#include "Disassembler.h"
#include <iomanip>
#include <sstream>
#include <array>
#include <algorithm>

namespace {

// Table of all 256 opcodes
std::array<OpcodeInfo, 256> BuildOpcodeTable() {
    std::array<OpcodeInfo, 256> table;
    // Default initialize to unknown
    for (int i = 0; i < 256; ++i) {
        table[i] = { "???", AddressingMode::Unknown, 1 };
    }

    auto set = [&](Byte op, const char* mnem, AddressingMode mode, int len) {
        table[op] = { mnem, mode, len };
    };

    // NOP, BRK, RTI
    set(0xEA, "NOP", AddressingMode::Implied, 1);
    set(0x00, "BRK", AddressingMode::Implied, 1);
    set(0x40, "RTI", AddressingMode::Implied, 1);

    // LDA
    set(0xA9, "LDA", AddressingMode::Immediate, 2);
    set(0xA5, "LDA", AddressingMode::ZeroPage, 2);
    set(0xB5, "LDA", AddressingMode::ZeroPageX, 2);
    set(0xAD, "LDA", AddressingMode::Absolute, 3);
    set(0xBD, "LDA", AddressingMode::AbsoluteX, 3);
    set(0xB9, "LDA", AddressingMode::AbsoluteY, 3);
    set(0xA1, "LDA", AddressingMode::IndirectX, 2);
    set(0xB1, "LDA", AddressingMode::IndirectY, 2);

    // LDX
    set(0xA2, "LDX", AddressingMode::Immediate, 2);
    set(0xA6, "LDX", AddressingMode::ZeroPage, 2);
    set(0xB6, "LDX", AddressingMode::ZeroPageY, 2);
    set(0xAE, "LDX", AddressingMode::Absolute, 3);
    set(0xBE, "LDX", AddressingMode::AbsoluteY, 3);

    // LDY
    set(0xA0, "LDY", AddressingMode::Immediate, 2);
    set(0xA4, "LDY", AddressingMode::ZeroPage, 2);
    set(0xB4, "LDY", AddressingMode::ZeroPageX, 2);
    set(0xAC, "LDY", AddressingMode::Absolute, 3);
    set(0xBC, "LDY", AddressingMode::AbsoluteX, 3);

    // STA
    set(0x85, "STA", AddressingMode::ZeroPage, 2);
    set(0x95, "STA", AddressingMode::ZeroPageX, 2);
    set(0x8D, "STA", AddressingMode::Absolute, 3);
    set(0x9D, "STA", AddressingMode::AbsoluteX, 3);
    set(0x99, "STA", AddressingMode::AbsoluteY, 3);
    set(0x81, "STA", AddressingMode::IndirectX, 2);
    set(0x91, "STA", AddressingMode::IndirectY, 2);

    // STX
    set(0x86, "STX", AddressingMode::ZeroPage, 2);
    set(0x96, "STX", AddressingMode::ZeroPageY, 2);
    set(0x8E, "STX", AddressingMode::Absolute, 3);

    // STY
    set(0x84, "STY", AddressingMode::ZeroPage, 2);
    set(0x94, "STY", AddressingMode::ZeroPageX, 2);
    set(0x8C, "STY", AddressingMode::Absolute, 3);

    // ADC
    set(0x69, "ADC", AddressingMode::Immediate, 2);
    set(0x65, "ADC", AddressingMode::ZeroPage, 2);
    set(0x75, "ADC", AddressingMode::ZeroPageX, 2);
    set(0x6D, "ADC", AddressingMode::Absolute, 3);
    set(0x7D, "ADC", AddressingMode::AbsoluteX, 3);
    set(0x79, "ADC", AddressingMode::AbsoluteY, 3);
    set(0x61, "ADC", AddressingMode::IndirectX, 2);
    set(0x71, "ADC", AddressingMode::IndirectY, 2);

    // SBC
    set(0xE9, "SBC", AddressingMode::Immediate, 2);
    set(0xE5, "SBC", AddressingMode::ZeroPage, 2);
    set(0xF5, "SBC", AddressingMode::ZeroPageX, 2);
    set(0xED, "SBC", AddressingMode::Absolute, 3);
    set(0xFD, "SBC", AddressingMode::AbsoluteX, 3);
    set(0xF9, "SBC", AddressingMode::AbsoluteY, 3);
    set(0xE1, "SBC", AddressingMode::IndirectX, 2);
    set(0xF1, "SBC", AddressingMode::IndirectY, 2);

    // CMP
    set(0xC9, "CMP", AddressingMode::Immediate, 2);
    set(0xC5, "CMP", AddressingMode::ZeroPage, 2);
    set(0xD5, "CMP", AddressingMode::ZeroPageX, 2);
    set(0xCD, "CMP", AddressingMode::Absolute, 3);
    set(0xDD, "CMP", AddressingMode::AbsoluteX, 3);
    set(0xD9, "CMP", AddressingMode::AbsoluteY, 3);
    set(0xC1, "CMP", AddressingMode::IndirectX, 2);
    set(0xD1, "CMP", AddressingMode::IndirectY, 2);

    // CPX
    set(0xE0, "CPX", AddressingMode::Immediate, 2);
    set(0xE4, "CPX", AddressingMode::ZeroPage, 2);
    set(0xEC, "CPX", AddressingMode::Absolute, 3);

    // CPY
    set(0xC0, "CPY", AddressingMode::Immediate, 2);
    set(0xC4, "CPY", AddressingMode::ZeroPage, 2);
    set(0xCC, "CPY", AddressingMode::Absolute, 3);

    // AND
    set(0x29, "AND", AddressingMode::Immediate, 2);
    set(0x25, "AND", AddressingMode::ZeroPage, 2);
    set(0x35, "AND", AddressingMode::ZeroPageX, 2);
    set(0x2D, "AND", AddressingMode::Absolute, 3);
    set(0x3D, "AND", AddressingMode::AbsoluteX, 3);
    set(0x39, "AND", AddressingMode::AbsoluteY, 3);
    set(0x21, "AND", AddressingMode::IndirectX, 2);
    set(0x31, "AND", AddressingMode::IndirectY, 2);

    // ORA
    set(0x09, "ORA", AddressingMode::Immediate, 2);
    set(0x05, "ORA", AddressingMode::ZeroPage, 2);
    set(0x15, "ORA", AddressingMode::ZeroPageX, 2);
    set(0x0D, "ORA", AddressingMode::Absolute, 3);
    set(0x1D, "ORA", AddressingMode::AbsoluteX, 3);
    set(0x19, "ORA", AddressingMode::AbsoluteY, 3);
    set(0x01, "ORA", AddressingMode::IndirectX, 2);
    set(0x11, "ORA", AddressingMode::IndirectY, 2);

    // EOR
    set(0x49, "EOR", AddressingMode::Immediate, 2);
    set(0x45, "EOR", AddressingMode::ZeroPage, 2);
    set(0x55, "EOR", AddressingMode::ZeroPageX, 2);
    set(0x4D, "EOR", AddressingMode::Absolute, 3);
    set(0x5D, "EOR", AddressingMode::AbsoluteX, 3);
    set(0x59, "EOR", AddressingMode::AbsoluteY, 3);
    set(0x41, "EOR", AddressingMode::IndirectX, 2);
    set(0x51, "EOR", AddressingMode::IndirectY, 2);

    // BIT
    set(0x24, "BIT", AddressingMode::ZeroPage, 2);
    set(0x2C, "BIT", AddressingMode::Absolute, 3);

    // INC
    set(0xE6, "INC", AddressingMode::ZeroPage, 2);
    set(0xF6, "INC", AddressingMode::ZeroPageX, 2);
    set(0xEE, "INC", AddressingMode::Absolute, 3);
    set(0xFE, "INC", AddressingMode::AbsoluteX, 3);
    set(0xE8, "INX", AddressingMode::Implied, 1);
    set(0xC8, "INY", AddressingMode::Implied, 1);

    // DEC
    set(0xC6, "DEC", AddressingMode::ZeroPage, 2);
    set(0xD6, "DEC", AddressingMode::ZeroPageX, 2);
    set(0xCE, "DEC", AddressingMode::Absolute, 3);
    set(0xDE, "DEC", AddressingMode::AbsoluteX, 3);
    set(0xCA, "DEX", AddressingMode::Implied, 1);
    set(0x88, "DEY", AddressingMode::Implied, 1);

    // ASL
    set(0x0A, "ASL", AddressingMode::Accumulator, 1);
    set(0x06, "ASL", AddressingMode::ZeroPage, 2);
    set(0x16, "ASL", AddressingMode::ZeroPageX, 2);
    set(0x0E, "ASL", AddressingMode::Absolute, 3);
    set(0x1E, "ASL", AddressingMode::AbsoluteX, 3);

    // LSR
    set(0x4A, "LSR", AddressingMode::Accumulator, 1);
    set(0x46, "LSR", AddressingMode::ZeroPage, 2);
    set(0x56, "LSR", AddressingMode::ZeroPageX, 2);
    set(0x4E, "LSR", AddressingMode::Absolute, 3);
    set(0x5E, "LSR", AddressingMode::AbsoluteX, 3);

    // ROL
    set(0x2A, "ROL", AddressingMode::Accumulator, 1);
    set(0x26, "ROL", AddressingMode::ZeroPage, 2);
    set(0x36, "ROL", AddressingMode::ZeroPageX, 2);
    set(0x2E, "ROL", AddressingMode::Absolute, 3);
    set(0x3E, "ROL", AddressingMode::AbsoluteX, 3);

    // ROR
    set(0x6A, "ROR", AddressingMode::Accumulator, 1);
    set(0x66, "ROR", AddressingMode::ZeroPage, 2);
    set(0x76, "ROR", AddressingMode::ZeroPageX, 2);
    set(0x6E, "ROR", AddressingMode::Absolute, 3);
    set(0x7E, "ROR", AddressingMode::AbsoluteX, 3);

    // Jumps & Calls
    set(0x4C, "JMP", AddressingMode::Absolute, 3);
    set(0x6C, "JMP", AddressingMode::Indirect, 3);
    set(0x20, "JSR", AddressingMode::Absolute, 3);
    set(0x60, "RTS", AddressingMode::Implied, 1);

    // Branches
    set(0xF0, "BEQ", AddressingMode::Relative, 2);
    set(0xD0, "BNE", AddressingMode::Relative, 2);
    set(0xB0, "BCS", AddressingMode::Relative, 2);
    set(0x90, "BCC", AddressingMode::Relative, 2);
    set(0x30, "BMI", AddressingMode::Relative, 2);
    set(0x10, "BPL", AddressingMode::Relative, 2);
    set(0x70, "BVS", AddressingMode::Relative, 2);
    set(0x50, "BVC", AddressingMode::Relative, 2);

    // Stack
    set(0x48, "PHA", AddressingMode::Implied, 1);
    set(0x68, "PLA", AddressingMode::Implied, 1);
    set(0x08, "PHP", AddressingMode::Implied, 1);
    set(0x28, "PLP", AddressingMode::Implied, 1);
    set(0xBA, "TSX", AddressingMode::Implied, 1);
    set(0x9A, "TXS", AddressingMode::Implied, 1);

    // Register Transfers
    set(0xAA, "TAX", AddressingMode::Implied, 1);
    set(0x8A, "TXA", AddressingMode::Implied, 1);
    set(0xA8, "TAY", AddressingMode::Implied, 1);
    set(0x98, "TYA", AddressingMode::Implied, 1);

    // Clear / Set Flags
    set(0x18, "CLC", AddressingMode::Implied, 1);
    set(0x38, "SEC", AddressingMode::Implied, 1);
    set(0x58, "CLI", AddressingMode::Implied, 1);
    set(0x78, "SEI", AddressingMode::Implied, 1);
    set(0xD8, "CLD", AddressingMode::Implied, 1);
    set(0xF8, "SED", AddressingMode::Implied, 1);
    set(0xB8, "CLV", AddressingMode::Implied, 1);

    return table;
}

const auto g_OpcodeTable = BuildOpcodeTable();

std::string HexByte(Byte b) {
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    return ss.str();
}

std::string HexWord(Word w) {
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << w;
    return ss.str();
}

} // anonymous namespace

const OpcodeInfo& Disassembler::GetOpcodeInfo(Byte opcode) {
    return g_OpcodeTable[opcode];
}

const char* Disassembler::GetAddressingModeName(AddressingMode mode) {
    switch (mode) {
        case AddressingMode::Implied:     return "Implied";
        case AddressingMode::Accumulator: return "Accumulator";
        case AddressingMode::Immediate:   return "Immediate";
        case AddressingMode::ZeroPage:    return "Zero Page";
        case AddressingMode::ZeroPageX:   return "Zero Page,X";
        case AddressingMode::ZeroPageY:   return "Zero Page,Y";
        case AddressingMode::Absolute:    return "Absolute";
        case AddressingMode::AbsoluteX:   return "Absolute,X";
        case AddressingMode::AbsoluteY:   return "Absolute,Y";
        case AddressingMode::Indirect:    return "Indirect";
        case AddressingMode::IndirectX:   return "(Indirect,X)";
        case AddressingMode::IndirectY:   return "(Indirect),Y";
        case AddressingMode::Relative:    return "Relative";
        default:                          return "Unknown";
    }
}

DisassembledInstruction Disassembler::Disassemble(const Memory& mem, Word address) {
    DisassembledInstruction inst;
    inst.address = address;
    inst.opcode = mem[address];

    const auto& info = GetOpcodeInfo(inst.opcode);
    inst.length = info.length;
    inst.mnemonic = info.mnemonic;
    inst.mode = info.mode;

    // Build bytes hex string
    std::stringstream hexStream;
    hexStream << HexByte(inst.opcode);
    for (int i = 1; i < inst.length; ++i) {
        Word operandAddr = static_cast<Word>(address + i);
        hexStream << " " << HexByte(mem[operandAddr]);
    }
    inst.bytesHex = hexStream.str();

    // Format operand string
    std::stringstream opStream;
    if (info.mode == AddressingMode::Implied) {
        // No operand
    } else if (info.mode == AddressingMode::Accumulator) {
        opStream << "A";
    } else if (info.mode == AddressingMode::Immediate) {
        Byte val = mem[static_cast<Word>(address + 1)];
        opStream << "#$" << HexByte(val);
    } else if (info.mode == AddressingMode::ZeroPage) {
        Byte zp = mem[static_cast<Word>(address + 1)];
        opStream << "$" << HexByte(zp);
    } else if (info.mode == AddressingMode::ZeroPageX) {
        Byte zp = mem[static_cast<Word>(address + 1)];
        opStream << "$" << HexByte(zp) << ",X";
    } else if (info.mode == AddressingMode::ZeroPageY) {
        Byte zp = mem[static_cast<Word>(address + 1)];
        opStream << "$" << HexByte(zp) << ",Y";
    } else if (info.mode == AddressingMode::Absolute) {
        Byte low = mem[static_cast<Word>(address + 1)];
        Byte high = mem[static_cast<Word>(address + 2)];
        Word target = (high << 8) | low;
        opStream << "$" << HexWord(target);
    } else if (info.mode == AddressingMode::AbsoluteX) {
        Byte low = mem[static_cast<Word>(address + 1)];
        Byte high = mem[static_cast<Word>(address + 2)];
        Word target = (high << 8) | low;
        opStream << "$" << HexWord(target) << ",X";
    } else if (info.mode == AddressingMode::AbsoluteY) {
        Byte low = mem[static_cast<Word>(address + 1)];
        Byte high = mem[static_cast<Word>(address + 2)];
        Word target = (high << 8) | low;
        opStream << "$" << HexWord(target) << ",Y";
    } else if (info.mode == AddressingMode::Indirect) {
        Byte low = mem[static_cast<Word>(address + 1)];
        Byte high = mem[static_cast<Word>(address + 2)];
        Word target = (high << 8) | low;
        opStream << "($" << HexWord(target) << ")";
    } else if (info.mode == AddressingMode::IndirectX) {
        Byte zp = mem[static_cast<Word>(address + 1)];
        opStream << "($" << HexByte(zp) << ",X)";
    } else if (info.mode == AddressingMode::IndirectY) {
        Byte zp = mem[static_cast<Word>(address + 1)];
        opStream << "($" << HexByte(zp) << "),Y";
    } else if (info.mode == AddressingMode::Relative) {
        SByte offset = static_cast<SByte>(mem[static_cast<Word>(address + 1)]);
        Word target = static_cast<Word>(address + 2 + offset);
        opStream << "$" << HexWord(target);
    } else {
        // Unknown opcode
        opStream << "$" << HexByte(inst.opcode);
    }

    inst.operand = opStream.str();
    if (!inst.operand.empty()) {
        inst.fullText = inst.mnemonic + " " + inst.operand;
    } else {
        inst.fullText = inst.mnemonic;
    }

    return inst;
}

std::vector<DisassembledInstruction> Disassembler::DisassembleRange(const Memory& mem, Word startAddress, int count) {
    std::vector<DisassembledInstruction> result;
    Word addr = startAddress;
    for (int i = 0; i < count; ++i) {
        auto inst = Disassemble(mem, addr);
        result.push_back(inst);
        addr = static_cast<Word>(addr + inst.length);
        if (addr < startAddress && i > 0) break; // Wrapped 64KB
    }
    return result;
}

std::vector<DisassembledInstruction> Disassembler::DisassembleAround(const Memory& mem, Word targetPC, int linesBefore, int linesAfter) {
    // To disassemble lines before targetPC, walk backwards by a reasonable byte offset (e.g. linesBefore * 3)
    // then walk forward to find an instruction stream that hits targetPC.
    int estimatedBackBytes = std::min(static_cast<int>(targetPC), linesBefore * 3);
    Word startAddr = static_cast<Word>(targetPC - estimatedBackBytes);

    std::vector<DisassembledInstruction> preceding;
    Word walk = startAddr;
    while (walk < targetPC) {
        auto inst = Disassemble(mem, walk);
        preceding.push_back(inst);
        walk = static_cast<Word>(walk + inst.length);
    }

    std::vector<DisassembledInstruction> result;
    // Take up to linesBefore from preceding
    if (static_cast<int>(preceding.size()) > linesBefore) {
        result.assign(preceding.end() - linesBefore, preceding.end());
    } else {
        result = preceding;
    }

    // Now disassemble starting at targetPC
    Word forwardAddr = targetPC;
    for (int i = 0; i <= linesAfter; ++i) {
        auto inst = Disassemble(mem, forwardAddr);
        result.push_back(inst);
        forwardAddr = static_cast<Word>(forwardAddr + inst.length);
    }

    return result;
}
