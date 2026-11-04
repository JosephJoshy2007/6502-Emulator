#include <gtest/gtest.h>
#include "m6502.h"

class M6502CompareTest : public ::testing::Test {
protected:
    Memory mem;
    CPU cpu;

    virtual void SetUp() override {
        cpu.Reset(mem);
    }

    virtual void TearDown() override {
    }
};

void VerifyUnchangedCompareFlags(const CPU& current, const CPU& original) {
    EXPECT_EQ(current.B, original.B);
    EXPECT_EQ(current.D, original.D);
    EXPECT_EQ(current.I, original.I);
    EXPECT_EQ(current.V, original.V);
}

// =============================================================================
// CMP - Core Mathematical Comparisons (A vs Value)
// =============================================================================

TEST_F(M6502CompareTest, CMPRegisterGreaterThanOperandSetsCarryOnly) {
    cpu.PC = 0x8000;
    cpu.A = 0x50;

    mem[0x8000] = CPU::INS_CMP_IMM;
    mem[0x8001] = 0x20;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_EQ(cpu.A, 0x50);
    EXPECT_TRUE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedCompareFlags(cpu, cpuCopy);
}

TEST_F(M6502CompareTest, CMPRegisterEqualsOperandSetsCarryAndZero) {
    cpu.PC = 0x8000;
    cpu.A = 0x40;

    mem[0x8000] = CPU::INS_CMP_IMM;
    mem[0x8001] = 0x40;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_EQ(cpu.A, 0x40);
    EXPECT_TRUE(cpu.C);
    EXPECT_TRUE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedCompareFlags(cpu, cpuCopy);
}

TEST_F(M6502CompareTest, CMPRegisterLessThanOperandSetsNegativeOnly) {
    cpu.PC = 0x8000;
    cpu.A = 0x10;

    mem[0x8000] = CPU::INS_CMP_IMM;
    mem[0x8001] = 0x30;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_EQ(cpu.A, 0x10);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_TRUE(cpu.N);
    VerifyUnchangedCompareFlags(cpu, cpuCopy);
}

TEST_F(M6502CompareTest, CMPRegisterLessThanOperandNegativeBitUnset) {
    cpu.PC = 0x8000;
    cpu.A = 0x05;

    mem[0x8000] = CPU::INS_CMP_IMM;
    mem[0x8001] = 0x90; // 0x05 - 0x90 = 0x75 (bit 7 is 0, so N is false)

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_EQ(cpu.A, 0x05);
    EXPECT_FALSE(cpu.C); // 0x05 < 0x90 (borrow occurred)
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N); // Result 0x75 has bit 7 clear!
    VerifyUnchangedCompareFlags(cpu, cpuCopy);
}

TEST_F(M6502CompareTest, CMPZeroAgainstZeroSetsCarryAndZero) {
    cpu.PC = 0x8000;
    cpu.A = 0x00;

    mem[0x8000] = CPU::INS_CMP_IMM;
    mem[0x8001] = 0x00;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_TRUE(cpu.C);
    EXPECT_TRUE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedCompareFlags(cpu, cpuCopy);
}

// =============================================================================
// CMP - Addressing Modes & Cycle Counts
// =============================================================================

TEST_F(M6502CompareTest, CMPZeroPageTakes3Cycles) {
    cpu.PC = 0x8000;
    cpu.A = 0x25;

    mem[0x8000] = CPU::INS_CMP_ZP;
    mem[0x8001] = 0x30;
    mem[0x0030] = 0x25;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 3);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 3);
    EXPECT_EQ(cpu.A, 0x25);
    EXPECT_TRUE(cpu.C);
    EXPECT_TRUE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedCompareFlags(cpu, cpuCopy);
}

TEST_F(M6502CompareTest, CMPZeroPageXTakes4CyclesAndWrapsZeroPage) {
    cpu.PC = 0x8000;
    cpu.A = 0x20;
    cpu.X = 0x10;

    mem[0x8000] = CPU::INS_CMP_ZP_X;
    mem[0x8001] = 0xF5;
    mem[0x0005] = 0x20;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_EQ(cpu.A, 0x20);
    EXPECT_TRUE(cpu.C);
    EXPECT_TRUE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedCompareFlags(cpu, cpuCopy);
}

TEST_F(M6502CompareTest, CMPAbsoluteTakes4Cycles) {
    cpu.PC = 0x8000;
    cpu.A = 0x7F;

    mem[0x8000] = CPU::INS_CMP_ABS;
    mem[0x8001] = 0x20;
    mem[0x8002] = 0x40;
    mem[0x4020] = 0x80;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_EQ(cpu.A, 0x7F);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_TRUE(cpu.N);
    VerifyUnchangedCompareFlags(cpu, cpuCopy);
}

TEST_F(M6502CompareTest, CMPAbsoluteXSamePageTakes4Cycles) {
    cpu.PC = 0x8000;
    cpu.A = 0x30;
    cpu.X = 0x05;

    mem[0x8000] = CPU::INS_CMP_ABS_X;
    mem[0x8001] = 0x10;
    mem[0x8002] = 0x30;
    mem[0x3015] = 0x20;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_EQ(cpu.A, 0x30);
    EXPECT_TRUE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedCompareFlags(cpu, cpuCopy);
}

TEST_F(M6502CompareTest, CMPAbsoluteXPageCrossingTakes5Cycles) {
    cpu.PC = 0x8000;
    cpu.A = 0x10;
    cpu.X = 0x05;

    mem[0x8000] = CPU::INS_CMP_ABS_X;
    mem[0x8001] = 0xFD;
    mem[0x8002] = 0x30;
    mem[0x3102] = 0x50;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 5);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 5);
    EXPECT_EQ(cpu.A, 0x10);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_TRUE(cpu.N);
    VerifyUnchangedCompareFlags(cpu, cpuCopy);
}

TEST_F(M6502CompareTest, CMPAbsoluteYSamePageTakes4Cycles) {
    cpu.PC = 0x8000;
    cpu.A = 0x55;
    cpu.Y = 0x04;

    mem[0x8000] = CPU::INS_CMP_ABS_Y;
    mem[0x8001] = 0x10;
    mem[0x8002] = 0x50;
    mem[0x5014] = 0x55;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_EQ(cpu.A, 0x55);
    EXPECT_TRUE(cpu.C);
    EXPECT_TRUE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedCompareFlags(cpu, cpuCopy);
}

TEST_F(M6502CompareTest, CMPAbsoluteYPageCrossingTakes5Cycles) {
    cpu.PC = 0x8000;
    cpu.A = 0x80;
    cpu.Y = 0x0A;

    mem[0x8000] = CPU::INS_CMP_ABS_Y;
    mem[0x8001] = 0xFA;
    mem[0x8002] = 0x50;
    mem[0x5104] = 0x10;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 5);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 5);
    EXPECT_EQ(cpu.A, 0x80);
    EXPECT_TRUE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedCompareFlags(cpu, cpuCopy);
}

TEST_F(M6502CompareTest, CMPIndirectXTakes6CyclesAndWrapsZeroPage) {
    cpu.PC = 0x8000;
    cpu.A = 0x44;
    cpu.X = 0x05;

    mem[0x8000] = CPU::INS_CMP_IND_X;
    mem[0x8001] = 0xFB;
    mem[0x0000] = 0x20;
    mem[0x0001] = 0x60;
    mem[0x6020] = 0x44;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 6);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 6);
    EXPECT_EQ(cpu.A, 0x44);
    EXPECT_TRUE(cpu.C);
    EXPECT_TRUE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedCompareFlags(cpu, cpuCopy);
}

TEST_F(M6502CompareTest, CMPIndirectYSamePageTakes5Cycles) {
    cpu.PC = 0x8000;
    cpu.A = 0x60;
    cpu.Y = 0x05;

    mem[0x8000] = CPU::INS_CMP_IND_Y;
    mem[0x8001] = 0x20;
    mem[0x0020] = 0x00;
    mem[0x0021] = 0x70;
    mem[0x7005] = 0x50;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 5);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 5);
    EXPECT_EQ(cpu.A, 0x60);
    EXPECT_TRUE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedCompareFlags(cpu, cpuCopy);
}

TEST_F(M6502CompareTest, CMPIndirectYPageCrossingTakes6Cycles) {
    cpu.PC = 0x8000;
    cpu.A = 0x10;
    cpu.Y = 0x05;

    mem[0x8000] = CPU::INS_CMP_IND_Y;
    mem[0x8001] = 0x20;
    mem[0x0020] = 0xFD;
    mem[0x0021] = 0x70;
    mem[0x7102] = 0x30;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 6);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 6);
    EXPECT_EQ(cpu.A, 0x10);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_TRUE(cpu.N);
    VerifyUnchangedCompareFlags(cpu, cpuCopy);
}

// =============================================================================
// CPX - Compare X Register (Immediate, Zero Page, Absolute)
// =============================================================================

TEST_F(M6502CompareTest, CPXImmediateGreaterThanTakes2Cycles) {
    cpu.PC = 0x8000;
    cpu.X = 0x80;

    mem[0x8000] = CPU::INS_CPX_IMM;
    mem[0x8001] = 0x7F;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_EQ(cpu.X, 0x80);
    EXPECT_TRUE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedCompareFlags(cpu, cpuCopy);
}

TEST_F(M6502CompareTest, CPXImmediateEqualTakes2Cycles) {
    cpu.PC = 0x8000;
    cpu.X = 0x33;

    mem[0x8000] = CPU::INS_CPX_IMM;
    mem[0x8001] = 0x33;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_EQ(cpu.X, 0x33);
    EXPECT_TRUE(cpu.C);
    EXPECT_TRUE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedCompareFlags(cpu, cpuCopy);
}

TEST_F(M6502CompareTest, CPXImmediateLessThanTakes2Cycles) {
    cpu.PC = 0x8000;
    cpu.X = 0x15;

    mem[0x8000] = CPU::INS_CPX_IMM;
    mem[0x8001] = 0x20;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_EQ(cpu.X, 0x15);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_TRUE(cpu.N);
    VerifyUnchangedCompareFlags(cpu, cpuCopy);
}

TEST_F(M6502CompareTest, CPXZeroPageTakes3Cycles) {
    cpu.PC = 0x8000;
    cpu.X = 0x60;

    mem[0x8000] = CPU::INS_CPX_ZP;
    mem[0x8001] = 0x15;
    mem[0x0015] = 0x60;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 3);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 3);
    EXPECT_EQ(cpu.X, 0x60);
    EXPECT_TRUE(cpu.C);
    EXPECT_TRUE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedCompareFlags(cpu, cpuCopy);
}

TEST_F(M6502CompareTest, CPXAbsoluteTakes4Cycles) {
    cpu.PC = 0x8000;
    cpu.X = 0x10;

    mem[0x8000] = CPU::INS_CPX_ABS;
    mem[0x8001] = 0x50;
    mem[0x8002] = 0x20;
    mem[0x2050] = 0x30;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_EQ(cpu.X, 0x10);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_TRUE(cpu.N);
    VerifyUnchangedCompareFlags(cpu, cpuCopy);
}

// =============================================================================
// CPY - Compare Y Register (Immediate, Zero Page, Absolute)
// =============================================================================

TEST_F(M6502CompareTest, CPYImmediateGreaterThanTakes2Cycles) {
    cpu.PC = 0x8000;
    cpu.Y = 0xAA;

    mem[0x8000] = CPU::INS_CPY_IMM;
    mem[0x8001] = 0x55;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_EQ(cpu.Y, 0xAA);
    EXPECT_TRUE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedCompareFlags(cpu, cpuCopy);
}

TEST_F(M6502CompareTest, CPYImmediateEqualTakes2Cycles) {
    cpu.PC = 0x8000;
    cpu.Y = 0x77;

    mem[0x8000] = CPU::INS_CPY_IMM;
    mem[0x8001] = 0x77;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_EQ(cpu.Y, 0x77);
    EXPECT_TRUE(cpu.C);
    EXPECT_TRUE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedCompareFlags(cpu, cpuCopy);
}

TEST_F(M6502CompareTest, CPYImmediateLessThanTakes2Cycles) {
    cpu.PC = 0x8000;
    cpu.Y = 0x02;

    mem[0x8000] = CPU::INS_CPY_IMM;
    mem[0x8001] = 0x08;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_EQ(cpu.Y, 0x02);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_TRUE(cpu.N);
    VerifyUnchangedCompareFlags(cpu, cpuCopy);
}

TEST_F(M6502CompareTest, CPYZeroPageTakes3Cycles) {
    cpu.PC = 0x8000;
    cpu.Y = 0x40;

    mem[0x8000] = CPU::INS_CPY_ZP;
    mem[0x8001] = 0x22;
    mem[0x0022] = 0x40;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 3);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 3);
    EXPECT_EQ(cpu.Y, 0x40);
    EXPECT_TRUE(cpu.C);
    EXPECT_TRUE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedCompareFlags(cpu, cpuCopy);
}

TEST_F(M6502CompareTest, CPYAbsoluteTakes4Cycles) {
    cpu.PC = 0x8000;
    cpu.Y = 0x50;

    mem[0x8000] = CPU::INS_CPY_ABS;
    mem[0x8001] = 0x00;
    mem[0x8002] = 0x30;
    mem[0x3000] = 0x10;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_EQ(cpu.Y, 0x50);
    EXPECT_TRUE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedCompareFlags(cpu, cpuCopy);
}