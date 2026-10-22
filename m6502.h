#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>


using Byte = unsigned char;
using Word = unsigned short;
using u32 = unsigned int;
using s32 = signed int;

struct Memory {

    static constexpr u32 MAX_SIZE = 1024 * 64;
    Byte Data[MAX_SIZE];

    void Initialize();

    Byte operator[](u32 address) const;

    Byte& operator[](u32 address);

};

struct CPU{

    Word PC;
    Byte SP;

    Byte A,X,Y;

    Byte C : 1;
    Byte Z : 1;
    Byte I : 1;
    Byte D : 1;
    Byte B : 1;
    Byte U : 1;
    Byte V : 1;
    Byte N : 1;

    void Reset(Memory& memory);

    void UseResetVector(Memory& memory);

    bool LoadProgram(const std::string& filepath, Memory& mem, CPU& cpu);

    Byte GetStatusRegister() const;

    void SetStatusRegister(Byte status);

    Byte FetchByte(Memory& memory, s32& cycles);

    Byte ReadByte(Memory& memory,s32& cycles, Word address);

    void WriteByte(Memory& memory, s32& cycles, Word address, Byte value);

    void WriteWord(Memory& memory, s32& cycles, Word address, Word value);

    void ZERO_AND_NEGATIVE_SET_STATUS(Byte& Register);

    void LOAD_REGISTER_ZERO_PAGE_WITH_X_OFFSET(Memory& memory, s32& cycles, Byte& Register);

    void LOAD_REGISTER_ZERO_PAGE_WITH_Y_OFFSET(Memory& memory, s32& cycles, Byte& Register);

    void LOAD_REGISTER_ABSOLUTE(Memory& memory, s32& cycles, Byte& Register);

    void LOAD_REGISTER_ABSOLUTE_WITH_X_OFFSET(Memory& memory, s32& cycles, Byte& Register);

    void LOAD_REGISTER_ABSOLUTE_WITH_Y_OFFSET(Memory& memory, s32& cycles, Byte& Register);

    void STORE_REGISTER_ZERO_PAGE(Memory& memory, s32& cycles, Byte& Register);

    void STORE_REGISTER_ZERO_PAGE_WITH_X_OFFSET(Memory& memory, s32& cycles, Byte& Register);

    void STORE_REGISTER_ZERO_PAGE_WITH_Y_OFFSET(Memory& memory, s32& cycles, Byte& Register);

    void STORE_REGISTER_ABSOLUTE(Memory& memory, s32& cycles, Byte& Register);

    static constexpr Byte 

        //LDA
        INS_LDA_IMM = 0xA9,
        INS_LDA_ZP = 0xA5,
        INS_LDA_ZP_X = 0xB5,
        INS_LDA_ABS_X = 0xBD,
        INS_LDA_ABS_Y = 0xB9,
        INS_LDA_ABS = 0xAD,
        INS_LDA_IND_X = 0xA1,
        INS_LDA_IND_Y = 0xB1,

        //LDX
        INS_LDX_IMM = 0xA2,
        INS_LDX_ZP = 0xA6,
        INS_LDX_ZP_Y = 0xB6,
        INS_LDX_ABS = 0xAE,
        INS_LDX_ABS_Y = 0xBE,

        //LDY
        INS_LDY_IMM = 0xA0,
        INS_LDY_ZP = 0xA4,
        INS_LDY_ZP_X = 0xB4,
        INS_LDY_ABS = 0xAC,
        INS_LDY_ABS_X = 0xBC,

        //STA
        INS_STA_ZP = 0x85,
        INS_STA_ZP_X = 0x95,
        INS_STA_ABS = 0x8D,
        INS_STA_ABS_X = 0x9D,
        INS_STA_ABS_Y = 0x99,
        INS_STA_IND_X = 0x81,
        INS_STA_IND_Y = 0x91,

        //STX
        INS_STX_ZP = 0x86,
        INS_STX_ZP_Y = 0x96,
        INS_STX_ABS = 0x8E,

        //STY
        INS_STY_ZP = 0x84,
        INS_STY_ZP_X = 0x94,
        INS_STY_ABS = 0x8C,

        INS_NOP = 0xEA,

        //Jump
        INS_JSR = 0x20,
        INS_JMP_ABS = 0x4C,
        INS_JMP_IND = 0x6C,
        INS_RTS = 0x60,
        
        //Stack
        INS_TSX = 0xBA,
        INS_TXS = 0x9A,
        INS_PHA = 0x48,
        INS_PLA = 0x68,
        INS_PHP = 0x08,
        INS_PLP = 0x28,

        //AND
        INS_AND_IMM = 0x29,
        INS_AND_ZP = 0x25,
        INS_AND_ZP_X = 0x35,
        INS_AND_ABS = 0x2D,
        INS_AND_ABS_X = 0x3D,
        INS_AND_ABS_Y = 0x39,
        INS_AND_IND_X = 0x21,
        INS_AND_IND_Y = 0x31,

        //EOR
        INS_EOR_IMM = 0x49,
        INS_EOR_ZP = 0x45,
        INS_EOR_ZP_X = 0x55,
        INS_EOR_ABS = 0x4D,
        INS_EOR_ABS_X = 0x5D,
        INS_EOR_ABS_Y = 0x59,
        INS_EOR_IND_X = 0x41,
        INS_EOR_IND_Y = 0x51,

        //ORA
        INS_ORA_IMM = 0x09,
        INS_ORA_ZP = 0x05,
        INS_ORA_ZP_X = 0x15,
        INS_ORA_ABS = 0x0D,
        INS_ORA_ABS_X = 0x1D,
        INS_ORA_ABS_Y = 0x19,
        INS_ORA_IND_X = 0x01,
        INS_ORA_IND_Y = 0x11,
        
        //BIT
        INS_BIT_ZP = 0x24,
        INS_BIT_ABS = 0x2C,
        
        //Register
        INS_TAX = 0xAA,
        INS_TAY = 0xA8,
        INS_TXA = 0x8A,
        INS_TYA = 0x98;

    s32 Execute(Memory& memory, s32 cycles);
    
};