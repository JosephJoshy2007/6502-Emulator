#include <gtest/gtest.h>
#include "m6502.h"

class M6502RegisterTest : public ::testing::Test {
protected:
    Memory mem;
    CPU cpu;

    virtual void SetUp() override {
        cpu.Reset(mem);
    }

    virtual void TearDown() override {
    }
};

void VerifyUnchangedFlagsForRegisters(const CPU& current, const CPU& original) {
    SCOPED_TRACE("Verifying unchanged status flags (C, B, D, I, V)");
    EXPECT_EQ(current.C, original.C);
    EXPECT_EQ(current.B, original.B);
    EXPECT_EQ(current.D, original.D);
    EXPECT_EQ(current.I, original.I);
    EXPECT_EQ(current.V, original.V);
}

TEST_F(M6502RegisterTest, TAXCanTransferAToX) {

    cpu.Reset(mem);
    cpu.PC = 0xFFFC;
    cpu.Z = cpu.N = true;
    cpu.A = 0x65;

    mem[0xFFFC] = CPU::INS_TAX;
    
    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);
    
    EXPECT_EQ(cyclesUsed, 2);

    EXPECT_EQ(cpu.A, cpu.X);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);

    VerifyUnchangedFlagsForRegisters(cpu, cpuCopy);
}

TEST_F(M6502RegisterTest, TAYCanTransferAToY) {

    cpu.Reset(mem);
    cpu.PC = 0xFFFC;
    cpu.Z = cpu.N = true;
    cpu.A = 0x65;

    mem[0xFFFC] = CPU::INS_TAY;
    
    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);
    
    EXPECT_EQ(cyclesUsed, 2);

    EXPECT_EQ(cpu.A, cpu.Y);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);

    VerifyUnchangedFlagsForRegisters(cpu, cpuCopy);
}

TEST_F(M6502RegisterTest, TXACanTransferXToA) {

    cpu.Reset(mem);
    cpu.PC = 0xFFFC;
    cpu.Z = cpu.N = true;
    cpu.X = 0x65;

    mem[0xFFFC] = CPU::INS_TXA;
    
    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);
    
    EXPECT_EQ(cyclesUsed, 2);

    EXPECT_EQ(cpu.A, cpu.X);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);

    VerifyUnchangedFlagsForRegisters(cpu, cpuCopy);
}

TEST_F(M6502RegisterTest, TYACanTransferYToA) {

    cpu.Reset(mem);
    cpu.PC = 0xFFFC;
    cpu.Z = cpu.N = true;
    cpu.Y = 0x65;

    mem[0xFFFC] = CPU::INS_TYA;
    
    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);
    
    EXPECT_EQ(cyclesUsed, 2);

    EXPECT_EQ(cpu.A, cpu.Y);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);

    VerifyUnchangedFlagsForRegisters(cpu, cpuCopy);
}