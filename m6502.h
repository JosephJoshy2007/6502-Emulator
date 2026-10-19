#pragma once

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
        INS_RTS = 0x60;

    s32 Execute(Memory& memory, s32 cycles);
    
};