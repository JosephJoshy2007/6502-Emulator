#include <gtest/gtest.h>
#include "m6502.h"

class M6502IncDecTest : public ::testing::Test {
protected:
    Memory mem;
    CPU cpu;

    virtual void SetUp() override {
        cpu.Reset(mem);
    }

    virtual void TearDown() override {
    }
};

void VerifyUnchangedFlagsForIncDec(const CPU& current, const CPU& original) {
    SCOPED_TRACE("Verifying unchanged status flags (C, B, D, I, V)");
    EXPECT_EQ(current.C, original.C);
    EXPECT_EQ(current.B, original.B);
    EXPECT_EQ(current.D, original.D);
    EXPECT_EQ(current.I, original.I);
    EXPECT_EQ(current.V, original.V);
}

TEST_F(M6502IncDecTest, INCZPCanIncrement) {

    cpu.Reset(mem);
    cpu.PC = 0xFFFC;
    cpu.Z = cpu.N = true;

    mem[0x64] = 0x10;
    mem[0xFFFC] = CPU::INS_INC_ZP;
    mem[0xFFFD] = 0x64;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 5);

    EXPECT_EQ(mem[0x64], 0x11);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);

    VerifyUnchangedFlagsForIncDec(cpu, cpuCopy);
    
}

TEST_F(M6502IncDecTest, INCZPXCanIncrement) {

    cpu.Reset(mem);
    cpu.PC = 0xFFFC;
    cpu.Z = cpu.N = true;

    mem[0x69] = 0x10;
    mem[0xFFFC] = CPU::INS_INC_ZP_X;
    mem[0xFFFD] = 0x64;
    cpu.X = 0x05;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 6);

    EXPECT_EQ(mem[0x69], 0x11);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);

    VerifyUnchangedFlagsForIncDec(cpu, cpuCopy);
    
}

TEST_F(M6502IncDecTest, INCABSCanIncrement) {

    cpu.Reset(mem);
    cpu.PC = 0xFFFC;
    cpu.Z = cpu.N = true;

    mem[0x8000] = 0x10;
    mem[0xFFFC] = CPU::INS_INC_ABS;
    mem[0xFFFD] = 0x00;
    mem[0xFFFE] = 0x80;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 6);

    EXPECT_EQ(mem[0x8000], 0x11);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);

    VerifyUnchangedFlagsForIncDec(cpu, cpuCopy);
    
}

TEST_F(M6502IncDecTest, INCABSXCanIncrement) {

    cpu.Reset(mem);
    cpu.PC = 0xFFFC;
    cpu.Z = cpu.N = true;

    mem[0x8005] = 0x10;
    mem[0xFFFC] = CPU::INS_INC_ABS_X;
    mem[0xFFFD] = 0x00;
    mem[0xFFFE] = 0x80;
    cpu.X = 0x05;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 7);

    EXPECT_EQ(mem[0x8005], 0x11);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);

    VerifyUnchangedFlagsForIncDec(cpu, cpuCopy);
    
}

TEST_F(M6502IncDecTest, INCXCanIncrement) {

    cpu.Reset(mem);
    cpu.PC = 0xFFFC;
    cpu.Z = cpu.N = true;

    mem[0xFFFC] = CPU::INS_INX;
    cpu.X = 0x05;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 7);

    EXPECT_EQ(cpu.X, 0x06);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);

    VerifyUnchangedFlagsForIncDec(cpu, cpuCopy);
    
}

TEST_F(M6502IncDecTest, INCYCanIncrement) {

    cpu.Reset(mem);
    cpu.PC = 0xFFFC;
    cpu.Z = cpu.N = true;

    mem[0xFFFC] = CPU::INS_INY;
    cpu.Y = 0x05;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 7);

    EXPECT_EQ(cpu.Y, 0x06);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);

    VerifyUnchangedFlagsForIncDec(cpu, cpuCopy);
    
}


TEST_F(M6502IncDecTest, DECZPCanDecrement) {

    cpu.Reset(mem);
    cpu.PC = 0xFFFC;
    cpu.Z = cpu.N = true;

    mem[0x64] = 0x10;
    mem[0xFFFC] = CPU::INS_DEC_ZP;
    mem[0xFFFD] = 0x64;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 5);

    EXPECT_EQ(mem[0x64], 0x0F);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);

    VerifyUnchangedFlagsForIncDec(cpu, cpuCopy);
    
}

TEST_F(M6502IncDecTest, DECZPXCanDecrement) {

    cpu.Reset(mem);
    cpu.PC = 0xFFFC;
    cpu.Z = cpu.N = true;

    mem[0x69] = 0x10;
    mem[0xFFFC] = CPU::INS_DEC_ZP_X;
    mem[0xFFFD] = 0x64;
    cpu.X = 0x05;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 6);

    EXPECT_EQ(mem[0x69], 0x0F);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);

    VerifyUnchangedFlagsForIncDec(cpu, cpuCopy);
    
}

TEST_F(M6502IncDecTest, DECABSCanDecrement) {

    cpu.Reset(mem);
    cpu.PC = 0xFFFC;
    cpu.Z = cpu.N = true;

    mem[0x8000] = 0x10;
    mem[0xFFFC] = CPU::INS_DEC_ABS;
    mem[0xFFFD] = 0x00;
    mem[0xFFFE] = 0x80;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 6);

    EXPECT_EQ(mem[0x8000], 0x0F);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);

    VerifyUnchangedFlagsForIncDec(cpu, cpuCopy);
    
}

TEST_F(M6502IncDecTest, DECABSXCanDecrement) {

    cpu.Reset(mem);
    cpu.PC = 0xFFFC;
    cpu.Z = cpu.N = true;

    mem[0x8005] = 0x10;
    mem[0xFFFC] = CPU::INS_DEC_ABS_X;
    mem[0xFFFD] = 0x00;
    mem[0xFFFE] = 0x80;
    cpu.X = 0x05;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 7);

    EXPECT_EQ(mem[0x8005], 0x0F);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);

    VerifyUnchangedFlagsForIncDec(cpu, cpuCopy);
    
}

TEST_F(M6502IncDecTest, DECXCanDecrement) {

    cpu.Reset(mem);
    cpu.PC = 0xFFFC;
    cpu.Z = cpu.N = true;

    mem[0xFFFC] = CPU::INS_DEX;
    cpu.X = 0x05;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.X, 0x04);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);

    VerifyUnchangedFlagsForIncDec(cpu, cpuCopy);
    
}

TEST_F(M6502IncDecTest, DECYCanDecrement) {

    cpu.Reset(mem);
    cpu.PC = 0xFFFC;
    cpu.Z = cpu.N = true;

    mem[0xFFFC] = CPU::INS_DEY;
    cpu.Y = 0x05;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.Y, 0x04);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);

    VerifyUnchangedFlagsForIncDec(cpu, cpuCopy);
    
}