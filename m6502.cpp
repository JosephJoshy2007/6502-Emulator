#include "m6502.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>

using Byte = unsigned char;
using SByte = char;
using Word = unsigned short;
using u32 = unsigned int;
using s32 = signed int;


void Memory::Initialize() {
    for (u32 i = 0; i < MAX_SIZE; i++) {
        Data[i] = 0;
    }
}

Byte Memory::operator[](u32 address) const {
    return Data[address];
}

Byte& Memory::operator[](u32 address) {
    return Data[address];
}

void CPU::Reset(Memory& memory) {

    PC = 0xFFFC;
    SP = 0xFF;
    A = X = Y = 0;
    C = Z = I = D = B = V = N = 0;

    memory.Initialize();
}

    
void CPU::UseResetVector(Memory& memory){

    PC = static_cast<Word>(memory[0xFFFC]) | (static_cast<Word>(memory[0xFFFD]) << 8);
    
}

bool CPU::LoadProgram(const std::string& filepath, Memory& mem, CPU& cpu) {
    // 1. Open the binary file at the end to determine its size
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file: " << filepath << "\n";
        return false;
    }

    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // File must at least contain the 2-byte vector
    if (fileSize < 2) {
        std::cerr << "Error: File is too small (needs at least 2 bytes for the vector).\n";
        return false;
    }

    // 2. Read the entire file into a buffer
    std::vector<uint8_t> buffer(fileSize);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), fileSize)) {
        std::cerr << "Error: Failed to read binary data from file.\n";
        return false;
    }

    // 3. Put the first 2 bytes into the reset vector ($FFFC and $FFFD)
    mem[0xFFFC] = buffer[0]; // Low byte
    mem[0xFFFD] = buffer[1]; // High byte

    // 4. Resolve the target start address from the vector
    uint16_t startAddress = static_cast<uint16_t>(buffer[0]) | 
                           (static_cast<uint16_t>(buffer[1]) << 8);

    // 5. Copy the remaining bytes starting from the target address
    for (size_t i = 2; i < buffer.size(); ++i) {
        // Prevent writing beyond 64KB boundary
        uint32_t currentAddress = startAddress + (i - 2);
        if (currentAddress > 0xFFFF) {
            std::cerr << "Warning: Program exceeds 64KB addressable memory space.\n";
            break;
        }
        mem[currentAddress] = buffer[i];
    }

    // 6. Point the CPU to the start address via the reset vector
    cpu.UseResetVector(mem);

    std::cout << "Successfully loaded " << (fileSize - 2) 
              << " bytes to 0x" << std::hex << startAddress << std::dec << "\n";
    return true;
}


Byte CPU::FetchByte(Memory& memory, s32& cycles) {
    Byte data = memory[PC];
    PC++;
    cycles--;
    return data;
}

Byte CPU::ReadByte(Memory& memory,s32& cycles, Word address) {
    cycles--;
    return memory[address];
}

Byte CPU::GetStatusRegister() const {
        return (C << 0) |
               (Z << 1) |
               (I << 2) |
               (D << 3) |
               (B << 4) |
               (1 << 5) | // Bit 5: ALWAYS 1 when pushed
               (V << 6) |
               (N << 7);
}

void CPU::SetStatusRegister(Byte status) {
        C = (status >> 0) & 1;
        Z = (status >> 1) & 1;
        I = (status >> 2) & 1;
        D = (status >> 3) & 1;
        // Bits 4 & 5 are not written back to hardware registers
        V = (status >> 6) & 1;
        N = (status >> 7) & 1;
}

void CPU::WriteByte(Memory& memory, s32& cycles, Word address, Byte value) {
    memory[address] = value;
    cycles--;
}

void CPU::WriteWord(Memory& memory, s32& cycles, Word address, Word value) {
    memory[address] = value & 0xFF; // Low byte
    memory[address + 1] = (value >> 8) & 0xFF; // High byte
    cycles -= 2;
}

void CPU::ZERO_AND_NEGATIVE_SET_STATUS(Byte& Register) {
    Z = (Register == 0);
    N = (Register & 0b10000000) > 0;
}

void CPU::LOAD_REGISTER_ZERO_PAGE_WITH_X_OFFSET(Memory& memory, s32& cycles, Byte& Register) {
    Byte addr = FetchByte(memory, cycles);
    addr += X;
    cycles--;
    Register = ReadByte(memory, cycles, addr);
    ZERO_AND_NEGATIVE_SET_STATUS(Register);
}

void CPU::LOAD_REGISTER_ZERO_PAGE_WITH_Y_OFFSET(Memory& memory, s32& cycles, Byte& Register) {
    Byte addr = FetchByte(memory, cycles);
    addr += Y;
    cycles--;
    Register = ReadByte(memory, cycles, addr);
    ZERO_AND_NEGATIVE_SET_STATUS(Register);
}

void CPU::LOAD_REGISTER_ABSOLUTE(Memory& memory, s32& cycles, Byte& Register) {
    Byte low = FetchByte(memory, cycles);
    Byte high = FetchByte(memory, cycles);
    Word address = (high << 8) | low;
    Register = ReadByte(memory, cycles, address);
    ZERO_AND_NEGATIVE_SET_STATUS(Register);
}

void CPU::LOAD_REGISTER_ABSOLUTE_WITH_X_OFFSET(Memory& memory, s32& cycles, Byte& Register) {
    Byte low = FetchByte(memory, cycles);
    Byte high = FetchByte(memory, cycles);
    Word address = (high << 8) | low;
    address += X;

    if ((address & 0xFF00) != (high << 8)) 
        cycles--; // Extra cycle for page crossing

    Register = ReadByte(memory, cycles, address);
    ZERO_AND_NEGATIVE_SET_STATUS(Register);
}

void CPU::LOAD_REGISTER_ABSOLUTE_WITH_Y_OFFSET(Memory& memory, s32& cycles, Byte& Register) {
    Byte low = FetchByte(memory, cycles);
    Byte high = FetchByte(memory, cycles);
    Word address = (high << 8) | low;
    address += Y;

    if ((address & 0xFF00) != (high << 8)) 
        cycles--; // Extra cycle for page crossing

    Register = ReadByte(memory, cycles, address);
    ZERO_AND_NEGATIVE_SET_STATUS(Register);
}


void CPU::STORE_REGISTER_ZERO_PAGE(Memory& memory, s32& cycles, Byte& Register) {
    Byte addr = FetchByte(memory, cycles);
    WriteByte(memory, cycles, addr, Register);
}

void CPU::STORE_REGISTER_ZERO_PAGE_WITH_X_OFFSET(Memory& memory, s32& cycles, Byte& Register) {
    Byte addr = FetchByte(memory, cycles);
    addr += X;
    cycles--;
    WriteByte(memory, cycles, addr, Register);
}

void CPU::STORE_REGISTER_ZERO_PAGE_WITH_Y_OFFSET(Memory& memory, s32& cycles, Byte& Register) {
    Byte addr = FetchByte(memory, cycles);
    addr += Y;
    cycles--;
    WriteByte(memory, cycles, addr, Register);
}

void CPU::STORE_REGISTER_ABSOLUTE(Memory& memory, s32& cycles, Byte& Register) {
    Byte low = FetchByte(memory, cycles);
    Byte high = FetchByte(memory, cycles);
    Word address = (high << 8) | low;
    WriteByte(memory, cycles, address, Register);
}

void CPU::BRANCH_IF_FLAG_SET(Memory& memory, s32& cycles, Byte& Flag) {
    Byte offset = FetchByte(memory, cycles);
    if (Flag){
        Word PCcopy = PC;
        PC += static_cast<SByte>(offset);
        cycles--;
        if ((PC & 0xFF00) != (PCcopy & 0xFF00))
            cycles--;
    }
}

void CPU::BRANCH_IF_FLAG_NOT_SET(Memory& memory, s32& cycles, Byte& Flag){
    Byte offset = FetchByte(memory, cycles);
    if (!Flag){
        Word PCcopy = PC;
        PC += static_cast<SByte>(offset);
        cycles--;
        if ((PC & 0xFF00) != (PCcopy & 0xFF00))
            cycles--;
    }
}


s32 CPU::Execute(Memory& memory, s32 cycles) {

    const s32 initial = cycles;

    while (cycles > 0) {

        printf("PC: %04X, A: %02X, X: %02X, Y: %02X, SP: %02X\n", PC, A, X, Y, SP);
        Byte opcode = FetchByte(memory,cycles);
        
        switch (opcode) {

            case INS_NOP:{ // NOP
                cycles--;
                break;
            }

            case INS_LDA_IMM: {
                Byte value = FetchByte(memory, cycles);
                A = value;
                ZERO_AND_NEGATIVE_SET_STATUS(A);

                break;
            }

            case INS_LDX_IMM: {
                Byte value = FetchByte(memory, cycles);
                X = value;
                ZERO_AND_NEGATIVE_SET_STATUS(X);

                break;
            }

            case INS_LDY_IMM: {
                Byte value = FetchByte(memory, cycles);
                Y = value;
                ZERO_AND_NEGATIVE_SET_STATUS(Y);

                break;
            }

            case INS_LDA_ZP: {
                Byte addr = FetchByte(memory, cycles);
                A = ReadByte(memory, cycles, addr);
                ZERO_AND_NEGATIVE_SET_STATUS(A);

                break;
            }

            case INS_LDX_ZP: {
                Byte addr = FetchByte(memory, cycles);
                X = ReadByte(memory, cycles, addr);
                ZERO_AND_NEGATIVE_SET_STATUS(X);

                break;
            }

            case INS_LDY_ZP: {
                Byte addr = FetchByte(memory, cycles);
                Y = ReadByte(memory, cycles, addr);
                ZERO_AND_NEGATIVE_SET_STATUS(Y);

                break;
            }

            case INS_LDA_ZP_X: {
                LOAD_REGISTER_ZERO_PAGE_WITH_X_OFFSET(memory, cycles, A);

                break;
            }

            case INS_LDX_ZP_Y: {
                LOAD_REGISTER_ZERO_PAGE_WITH_Y_OFFSET(memory, cycles, X);

                break;
            }

            case INS_LDY_ZP_X: {
                LOAD_REGISTER_ZERO_PAGE_WITH_X_OFFSET(memory, cycles, Y);

                break;
            }

            case INS_LDA_ABS: {
                LOAD_REGISTER_ABSOLUTE(memory, cycles, A);

                break;
            }

            case INS_LDX_ABS: {
                LOAD_REGISTER_ABSOLUTE(memory, cycles, X);

                break;
            }

            case INS_LDY_ABS: {
                LOAD_REGISTER_ABSOLUTE(memory, cycles, Y);

                break;
            }

            case INS_LDA_ABS_X: {
                LOAD_REGISTER_ABSOLUTE_WITH_X_OFFSET(memory, cycles, A);

                break;
            }

            case INS_LDA_ABS_Y: {
                LOAD_REGISTER_ABSOLUTE_WITH_Y_OFFSET(memory, cycles, A);

                break;
            }
            case INS_LDX_ABS_Y: {
                LOAD_REGISTER_ABSOLUTE_WITH_Y_OFFSET(memory, cycles, X);

                break;
            }

            case INS_LDY_ABS_X: {
                LOAD_REGISTER_ABSOLUTE_WITH_X_OFFSET(memory, cycles, Y);

                break;
            }

            case INS_LDA_IND_X: {
                Byte zpAddr = FetchByte(memory, cycles);
                Byte effectiveAddrLow = ReadByte(memory, cycles, (zpAddr + X) & 0xFF);
                Byte effectiveAddrHigh = ReadByte(memory, cycles, (zpAddr + X + 1) & 0xFF);
                Word effectiveAddress = (effectiveAddrHigh << 8) | effectiveAddrLow;
                cycles--;

                A = ReadByte(memory, cycles, effectiveAddress);
                ZERO_AND_NEGATIVE_SET_STATUS(A);

                break;
            }

            case INS_LDA_IND_Y: {
                Byte zpAddr = FetchByte(memory, cycles);
                Byte effectiveAddrLow = ReadByte(memory, cycles, zpAddr);
                Byte effectiveAddrHigh = ReadByte(memory, cycles, (zpAddr + 1) & 0xFF);
                Word effectiveAddress = (effectiveAddrHigh << 8) | effectiveAddrLow;
                effectiveAddress += Y;

                if ((effectiveAddress & 0xFF00) != (effectiveAddrHigh << 8)) 
                    cycles--; // Extra cycle for page crossing

                A = ReadByte(memory, cycles, effectiveAddress);
                ZERO_AND_NEGATIVE_SET_STATUS(A);

                break;
            }




            case INS_STA_ZP: {
                STORE_REGISTER_ZERO_PAGE(memory, cycles, A);
                break;
            }

            case INS_STX_ZP: {
                STORE_REGISTER_ZERO_PAGE(memory, cycles, X);
                break;
            }

            case INS_STY_ZP: {
                STORE_REGISTER_ZERO_PAGE(memory, cycles, Y);
                break;
            }

            case INS_STA_ZP_X: {
                STORE_REGISTER_ZERO_PAGE_WITH_X_OFFSET(memory, cycles, A);
                break;
            }

            case INS_STX_ZP_Y: {
                STORE_REGISTER_ZERO_PAGE_WITH_Y_OFFSET(memory, cycles, X);
                break;
            }

            case INS_STY_ZP_X: {
                STORE_REGISTER_ZERO_PAGE_WITH_X_OFFSET(memory, cycles, Y);
                break;
            }

            case INS_STA_ABS: {
                STORE_REGISTER_ABSOLUTE(memory, cycles, A);
                break;
            }

            case INS_STX_ABS: {
                STORE_REGISTER_ABSOLUTE(memory, cycles, X);
                break;
            }

            case INS_STY_ABS: {
                STORE_REGISTER_ABSOLUTE(memory, cycles, Y);
                break;
            }

            case INS_STA_ABS_X: {
                Byte low = FetchByte(memory, cycles);
                Byte high = FetchByte(memory, cycles);
                Word address = (high << 8) | low;
                address += X;
                cycles--;

                WriteByte(memory, cycles, address, A);

                break;
            }

            case INS_STA_ABS_Y: {
                Byte low = FetchByte(memory, cycles);
                Byte high = FetchByte(memory, cycles);
                Word address = (high << 8) | low;
                address += Y;
                cycles--;

                WriteByte(memory, cycles, address, A);

                break;
            }

            case INS_STA_IND_X: {
                Byte zpAddr = FetchByte(memory, cycles);
                Byte effectiveAddrLow = ReadByte(memory, cycles, (zpAddr + X) & 0xFF);
                Byte effectiveAddrHigh = ReadByte(memory, cycles, (zpAddr + X + 1) & 0xFF);
                Word effectiveAddress = (effectiveAddrHigh << 8) | effectiveAddrLow;
                cycles--;

                WriteByte(memory, cycles, effectiveAddress, A);

                break;
            }

            case INS_STA_IND_Y: {
                Byte zpAddr = FetchByte(memory, cycles);
                Byte effectiveAddrLow = ReadByte(memory, cycles, zpAddr);
                Byte effectiveAddrHigh = ReadByte(memory, cycles, (zpAddr + 1) & 0xFF);
                Word effectiveAddress = (effectiveAddrHigh << 8) | effectiveAddrLow;
                effectiveAddress += Y;

                cycles--;

                WriteByte(memory, cycles, effectiveAddress, A);

                break;
            }




            case INS_JMP_ABS: {
                Byte low = FetchByte(memory, cycles);
                Byte high = FetchByte(memory, cycles);
                Word address = (high << 8) | low;
                PC = address;

                break;
            }

            case INS_JMP_IND: {
                Byte low = FetchByte(memory, cycles);
                Byte high = FetchByte(memory, cycles);
                Word pointerAddress = (high << 8) | low;

                // Handle the 6502 bug where the high byte wraps around on page boundary
                Byte effectiveAddrLow = ReadByte(memory, cycles, pointerAddress);
                Byte effectiveAddrHigh = ReadByte(memory, cycles, (pointerAddress & 0xFF00) | ((pointerAddress + 1) & 0x00FF));
                Word effectiveAddress = (effectiveAddrHigh << 8) | effectiveAddrLow;

                PC = effectiveAddress;

                break;
            }

            case INS_JSR: {
                Byte low = FetchByte(memory, cycles);
                Byte high = FetchByte(memory, cycles);
                Word address = (high << 8) | low;

                // Push return address onto stack
                Word returnAddress = PC - 1; // Address of the next instruction after JSR
                WriteByte(memory,cycles, SP | 0x0100, (returnAddress >> 8) & 0xFF); // Push high byte
                SP--;
                WriteByte(memory,cycles, SP | 0x0100, returnAddress & 0xFF); // Push low byte
                SP--;

                PC = address;
                cycles--; // JSR takes an additional cycle for the jump

                break;
            }

            case INS_RTS: {
                // Pull return address from stack
                SP++;
                Byte low = ReadByte(memory, cycles, SP | 0x0100); // Pull low byte
                SP++;
                Byte high = ReadByte(memory, cycles, SP | 0x0100); // Pull high byte
                Word returnAddress = (high << 8) | low;

                PC = returnAddress + 1; // Set PC to the instruction after JSR
                cycles -= 3;

                break;
            }




            case INS_TSX: {
                X = SP;
                ZERO_AND_NEGATIVE_SET_STATUS(X);
                cycles--;
                break;
            }

            case INS_TXS: {
                SP = X;
                cycles--;
                break;
            }

            case INS_PHA: {
                WriteByte(memory, cycles, SP | 0x100, A);
                SP--;
                cycles--;
                break;
            }

            case INS_PLA: {
                SP++;
                cycles--;
                A = ReadByte(memory, cycles, SP | 0x100);
                ZERO_AND_NEGATIVE_SET_STATUS(A);
                cycles--;
                break;
            }

            case INS_PHP: {
                Byte status = GetStatusRegister();
                WriteByte(memory, cycles, SP | 0x100, status);
                SP--;
                cycles--;
                break;
            }

            case INS_PLP: {
                SP++;
                Byte status = ReadByte(memory, cycles, SP | 0x100);
                SetStatusRegister(status);
                cycles -=2;
                break;
            }



            case INS_AND_IMM:
            case INS_EOR_IMM:
            case INS_ORA_IMM: {
                Byte value = FetchByte(memory, cycles);
                if (opcode == INS_AND_IMM) {
                    A &= value;
                } else if (opcode == INS_EOR_IMM) {
                    A ^= value;
                } else if (opcode == INS_ORA_IMM) {
                    A |= value;
                }
                ZERO_AND_NEGATIVE_SET_STATUS(A);
                break;
            }

            case INS_AND_ZP:
            case INS_EOR_ZP:
            case INS_ORA_ZP: {
                Byte addr = FetchByte(memory, cycles);
                Byte value = ReadByte(memory, cycles, addr);
                if (opcode == INS_AND_ZP) {
                    A &= value;
                } else if (opcode == INS_EOR_ZP) {
                    A ^= value;
                } else if (opcode == INS_ORA_ZP) {
                    A |= value;
                }
                ZERO_AND_NEGATIVE_SET_STATUS(A);
                break;
            }

            case INS_AND_ZP_X:
            case INS_EOR_ZP_X:
            case INS_ORA_ZP_X: {
                Byte addr = FetchByte(memory, cycles);
                addr += X;
                cycles--;
                Byte value = ReadByte(memory, cycles, addr);
                if (opcode == INS_AND_ZP_X) {
                    A &= value;
                } else if (opcode == INS_EOR_ZP_X) {
                    A ^= value;
                } else if (opcode == INS_ORA_ZP_X) {
                    A |= value;
                }
                ZERO_AND_NEGATIVE_SET_STATUS(A);
                break;
            }

            case INS_AND_ABS:
            case INS_EOR_ABS:
            case INS_ORA_ABS: {
                Byte low = FetchByte(memory, cycles);
                Byte high = FetchByte(memory, cycles);
                Word address = (high << 8) | low;
                Byte value = ReadByte(memory, cycles, address);
                if (opcode == INS_AND_ABS) {
                    A &= value;
                } else if (opcode == INS_EOR_ABS) {
                    A ^= value;
                } else if (opcode == INS_ORA_ABS) {
                    A |= value;
                }
                ZERO_AND_NEGATIVE_SET_STATUS(A);
                break;
            }

            case INS_AND_ABS_X:
            case INS_EOR_ABS_X:
            case INS_ORA_ABS_X: {
                Byte low = FetchByte(memory, cycles);
                Byte high = FetchByte(memory, cycles);
                Word address = (high << 8) | low;
                address += X;
                Byte value = ReadByte(memory, cycles, address);
                if (((address) & 0xFF00) != (high << 8))
                    cycles--; // Extra cycle for page crossing
                if (opcode == INS_AND_ABS_X) {
                    A &= value;
                } else if (opcode == INS_EOR_ABS_X) {
                    A ^= value;
                } else if (opcode == INS_ORA_ABS_X) {
                    A |= value;
                }
                ZERO_AND_NEGATIVE_SET_STATUS(A);
                break;
            }

            case INS_AND_ABS_Y:
            case INS_EOR_ABS_Y:
            case INS_ORA_ABS_Y: {
                Byte low = FetchByte(memory, cycles);
                Byte high = FetchByte(memory, cycles);
                Word address = (high << 8) | low;
                address += Y;
                Byte value = ReadByte(memory, cycles, address);
                if (((address) & 0xFF00) != (high << 8))
                    cycles--; // Extra cycle for page crossing
                if (opcode == INS_AND_ABS_Y) {
                    A &= value;
                } else if (opcode == INS_EOR_ABS_Y) {
                    A ^= value;
                } else if (opcode == INS_ORA_ABS_Y) {
                    A |= value;
                }
                ZERO_AND_NEGATIVE_SET_STATUS(A);
                break;
            }

            case INS_AND_IND_X:
            case INS_EOR_IND_X:
            case INS_ORA_IND_X: {
                Byte zpAddr = FetchByte(memory, cycles);
                Byte effectiveAddrLow = ReadByte(memory, cycles, (zpAddr + X) & 0xFF);
                Byte effectiveAddrHigh = ReadByte(memory, cycles, (zpAddr + X + 1) & 0xFF);
                Word effectiveAddress = (effectiveAddrHigh << 8) | effectiveAddrLow;
                cycles--;

                Byte value = ReadByte(memory, cycles, effectiveAddress);
                if (opcode == INS_AND_IND_X) {
                    A &= value;
                } else if (opcode == INS_EOR_IND_X) {
                    A ^= value;
                } else if (opcode == INS_ORA_IND_X) {
                    A |= value;
                }
                ZERO_AND_NEGATIVE_SET_STATUS(A);
                break;
            }

            case INS_AND_IND_Y:
            case INS_EOR_IND_Y:
            case INS_ORA_IND_Y: {
                Byte zpAddr = FetchByte(memory, cycles);
                Byte effectiveAddrLow = ReadByte(memory, cycles, zpAddr);
                Byte effectiveAddrHigh = ReadByte(memory, cycles, (zpAddr + 1) & 0xFF);
                Word effectiveAddress = (effectiveAddrHigh << 8) | effectiveAddrLow;
                effectiveAddress += Y;

                if ((effectiveAddress & 0xFF00) != (effectiveAddrHigh << 8)) 
                    cycles--; // Extra cycle for page crossing

                Byte value = ReadByte(memory, cycles, effectiveAddress);
                if (opcode == INS_AND_IND_Y) {
                    A &= value;
                } else if (opcode == INS_EOR_IND_Y) {
                    A ^= value;
                } else if (opcode == INS_ORA_IND_Y) {
                    A |= value;
                }
                ZERO_AND_NEGATIVE_SET_STATUS(A);
                break;
            }

            case INS_BIT_ZP: {
                Byte addr = FetchByte(memory, cycles);
                Byte value = ReadByte(memory, cycles, addr);
                Z = (A & value) == 0;
                N = (value & 0b10000000) > 0;
                V = (value & 0b01000000) > 0;
                break;
            }

            case INS_BIT_ABS: {
                Byte low = FetchByte(memory, cycles);
                Byte high = FetchByte(memory, cycles);
                Word address = (high << 8) | low;
                Byte value = ReadByte(memory, cycles, address);
                Z = (A & value) == 0;
                N = (value & 0b10000000) > 0;
                V = (value & 0b01000000) > 0;
                break;
            }



            case INS_TAX: {
                X = A;
                cycles--;
                ZERO_AND_NEGATIVE_SET_STATUS(X);
                break;
            }

            case INS_TAY: {
                Y = A;
                cycles--;
                ZERO_AND_NEGATIVE_SET_STATUS(Y);
                break;
            }

            case INS_TXA: {
                A = X;
                cycles--;
                ZERO_AND_NEGATIVE_SET_STATUS(A);
                break;
            }

            case INS_TYA: {
                A = Y;
                cycles--;
                ZERO_AND_NEGATIVE_SET_STATUS(A);
                break;
            }



            case INS_INC_ZP: {
                Byte addr = FetchByte(memory, cycles);
                cycles--;
                memory[addr]++;
                cycles -= 2;
                ZERO_AND_NEGATIVE_SET_STATUS(memory[addr]);
                break;
            }

            case INS_INC_ZP_X: {
                Byte addr = FetchByte(memory, cycles);
                cycles--;
                addr += X;
                cycles--;
                memory[addr]++;
                cycles -= 2;
                ZERO_AND_NEGATIVE_SET_STATUS(memory[addr]);
                break;
            }

            case INS_INC_ABS: {
                Byte low = FetchByte(memory, cycles);
                Byte high = FetchByte(memory, cycles);
                cycles--;
                Word addr = (high << 8) | low;
                cycles--;
                memory[addr]++;
                cycles--;
                ZERO_AND_NEGATIVE_SET_STATUS(memory[addr]);
            }

            case INS_INC_ABS_X: {
                Byte low = FetchByte(memory, cycles);
                Byte high = FetchByte(memory, cycles);
                cycles--;
                Word addr = (high << 8) | low;
                cycles--;
                addr += X;
                cycles--;
                memory[addr]++;
                cycles--;
                ZERO_AND_NEGATIVE_SET_STATUS(memory[addr]);
            }

            case INS_INX: {
                X++;
                cycles--;
                ZERO_AND_NEGATIVE_SET_STATUS(X);
                break;
            }

            case INS_INY: {
                Y++;
                cycles--;
                ZERO_AND_NEGATIVE_SET_STATUS(Y);
                break;
            }

            case INS_DEC_ZP: {
                Byte addr = FetchByte(memory, cycles);
                cycles--;
                memory[addr]--;
                cycles -= 2;
                ZERO_AND_NEGATIVE_SET_STATUS(memory[addr]);
                break;
            }

            case INS_DEC_ZP_X: {
                Byte addr = FetchByte(memory, cycles);
                cycles--;
                addr += X;
                cycles--;
                memory[addr]--;
                cycles -= 2;
                ZERO_AND_NEGATIVE_SET_STATUS(memory[addr]);
                break;
            }

            case INS_DEC_ABS: {
                Byte low = FetchByte(memory, cycles);
                Byte high = FetchByte(memory, cycles);
                cycles--;
                Word addr = (high << 8) | low;
                cycles--;
                memory[addr]--;
                cycles--;
                ZERO_AND_NEGATIVE_SET_STATUS(memory[addr]);
                break;
            }

            case INS_DEC_ABS_X: {
                Byte low = FetchByte(memory, cycles);
                Byte high = FetchByte(memory, cycles);
                cycles--;
                Word addr = (high << 8) | low;
                cycles--;
                addr += X;
                cycles--;
                memory[addr]--;
                cycles--;
                ZERO_AND_NEGATIVE_SET_STATUS(memory[addr]);
                break;
            }

            case INS_DEX: {
                X--;
                cycles--;
                ZERO_AND_NEGATIVE_SET_STATUS(X);
                break;
            }

            case INS_DEY: {
                Y--;
                cycles--;
                ZERO_AND_NEGATIVE_SET_STATUS(Y);
                break;
            }



            case INS_BEQ: {
                BRANCH_IF_FLAG_SET(memory, cycles, Z);
                break;
            }

            case INS_BCC: {
                BRANCH_IF_FLAG_NOT_SET(memory, cycles, C);
                break;
            }

            case INS_BCS: {
                BRANCH_IF_FLAG_SET(memory, cycles, C);
                break;
            }

            case INS_BMI: {
                BRANCH_IF_FLAG_SET(memory, cycles, N);
                break;
            }

            case INS_BNE: {
                BRANCH_IF_FLAG_NOT_SET(memory, cycles, Z);
                break;
            }

            case INS_BPL: {
                BRANCH_IF_FLAG_NOT_SET(memory, cycles, N);
                break;
            }

            case INS_BVC: {
                BRANCH_IF_FLAG_NOT_SET(memory, cycles, V);
                break;
            }

            case INS_BVS: {
                BRANCH_IF_FLAG_SET(memory, cycles, V);
                break;
            }

            case INS_CLC: {
                C = 0;
                cycles--;
                break;
            }

            case INS_CLI: {
                I = 0;
                cycles--;
                break;
            }

            case INS_CLV: {
                V = 0;
                cycles--;
                break;
            }

            case INS_SEC: {
                C = 1;
                cycles--;
                break;
            }

            case INS_SED: {
                D = 1;
                cycles--;
                break;
            }

            case INS_SEI: {
                I = 1;
                cycles--;
                break;
            }

            case INS_CLD: {
                D = 0;
                cycles--;
                break;
            }



            default:
                printf("Unknown opcode: 0x%02X at PC: 0x%04X\n", opcode, PC-1);
                break;
        }
        
    }
    printf("PC: %04X, A: %02X, X: %02X, Y: %02X, SP: %02X\n", PC, A, X, Y, SP);
    const s32 cyclesUsed = initial - cycles;
    return cyclesUsed;
}