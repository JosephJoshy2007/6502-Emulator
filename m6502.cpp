#include "m6502.h"
#include <stdio.h>
#include <stdlib.h>

using Byte = unsigned char;
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


s32 CPU::Execute(Memory& memory, s32 cycles) {

    const s32 initial = cycles;

    while (cycles > 0) {
        Byte opcode = FetchByte(memory,cycles);
        
        switch (opcode) {

            case INS_NOP:{ // NOP
                cycles--;
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

            default:
                printf("Unknown opcode: 0x%02X at PC: 0x%04X\n", opcode, PC-1);
                break;
        }
        
    }
    const s32 cyclesUsed = initial - cycles;
    return cyclesUsed;
}