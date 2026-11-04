#include <gtest/gtest.h>
#include "m6502.h"

class M6502SetClearTest : public ::testing::Test {
protected:
    Memory mem;
    CPU cpu;

    virtual void SetUp() override {
        cpu.Reset(mem);
    }

    virtual void TearDown() override {
    }
};

void VerifyOnlyTargetFlagChanged(const CPU& current, const CPU& original, char modifiedFlag) {
    if (modifiedFlag != 'C') EXPECT_EQ(current.C, original.C);
    if (modifiedFlag != 'I') EXPECT_EQ(current.I, original.I);
    if (modifiedFlag != 'V') EXPECT_EQ(current.V, original.V);
    if (modifiedFlag != 'D') EXPECT_EQ(current.D, original.D);
    if (modifiedFlag != 'Z') EXPECT_EQ(current.Z, original.Z);
    if (modifiedFlag != 'N') EXPECT_EQ(current.N, original.N);
    if (modifiedFlag != 'B') EXPECT_EQ(current.B, original.B);
}

TEST_F(M6502SetClearTest, CLCClearsCarryFlag) {
    cpu.PC = 0x8000;
    cpu.C = true;

    mem[0x8000] = CPU::INS_CLC;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8001);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_FALSE(cpu.C);
    VerifyOnlyTargetFlagChanged(cpu, cpuCopy, 'C');
}

TEST_F(M6502SetClearTest, SECSetsCarryFlag) {
    cpu.PC = 0x8000;
    cpu.C = false;

    mem[0x8000] = CPU::INS_SEC;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8001);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_TRUE(cpu.C);
    VerifyOnlyTargetFlagChanged(cpu, cpuCopy, 'C');
}

// ---------------------------------------------------------------------------
// CLI - Clear Interrupt Disable (0x58)
// ---------------------------------------------------------------------------
TEST_F(M6502SetClearTest, CLIClearsInterruptDisableFlag) {
    cpu.PC = 0x8000;
    cpu.I = true;

    mem[0x8000] = CPU::INS_CLI;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8001);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_FALSE(cpu.I);
    VerifyOnlyTargetFlagChanged(cpu, cpuCopy, 'I');
}

// ---------------------------------------------------------------------------
// SEI - Set Interrupt Disable (0x78)
// ---------------------------------------------------------------------------
TEST_F(M6502SetClearTest, SEISetsInterruptDisableFlag) {
    cpu.PC = 0x8000;
    cpu.I = false;

    mem[0x8000] = CPU::INS_SEI;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8001);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_TRUE(cpu.I);
    VerifyOnlyTargetFlagChanged(cpu, cpuCopy, 'I');
}

// ---------------------------------------------------------------------------
// CLD - Clear Decimal Mode (0xD8)
// ---------------------------------------------------------------------------
TEST_F(M6502SetClearTest, CLDClearsDecimalFlag) {
    cpu.PC = 0x8000;
    cpu.D = true;

    mem[0x8000] = CPU::INS_CLD;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8001);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_FALSE(cpu.D);
    VerifyOnlyTargetFlagChanged(cpu, cpuCopy, 'D');
}

// ---------------------------------------------------------------------------
// SED - Set Decimal Mode (0xF8)
// ---------------------------------------------------------------------------
TEST_F(M6502SetClearTest, SEDSetsDecimalFlag) {
    cpu.PC = 0x8000;
    cpu.D = false;

    mem[0x8000] = CPU::INS_SED;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8001);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_TRUE(cpu.D);
    VerifyOnlyTargetFlagChanged(cpu, cpuCopy, 'D');
}

// ---------------------------------------------------------------------------
// CLV - Clear Overflow (0xB8)
// ---------------------------------------------------------------------------
TEST_F(M6502SetClearTest, CLVClearsOverflowFlag) {
    cpu.PC = 0x8000;
    cpu.V = true;

    mem[0x8000] = CPU::INS_CLV;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8001);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_FALSE(cpu.V);
    VerifyOnlyTargetFlagChanged(cpu, cpuCopy, 'V');
}