#include <gtest/gtest.h>
#include "m6502.h"

class M6502LogicalShiftRightTest : public ::testing::Test {
protected:
    Memory mem;
    CPU cpu;

    virtual void SetUp() override {
        cpu.Reset(mem);
    }

    virtual void TearDown() override {
    }
};

void VerifyUnchangedNonShiftFlags(const CPU& current, const CPU& original) {
    EXPECT_EQ(current.B, original.B);
    EXPECT_EQ(current.D, original.D);
    EXPECT_EQ(current.I, original.I);
    EXPECT_EQ(current.V, original.V); // LSR does not touch overflow
}

// ---------------------------------------------------------------------------
// Flag & Boundary Behavior (tested via LSR Accumulator)
// ---------------------------------------------------------------------------

TEST_F(M6502LogicalShiftRightTest, LSRAccumulatorShiftsZeroYieldsZeroSetsZeroFlag) {
    cpu.PC = 0x8000;
    cpu.A = 0x00;
    cpu.C = true; // Pre-existing carry should be cleared by bit 0 (0)

    mem[0x8000] = CPU::INS_LSR_A;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8001);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_FALSE(cpu.C);  // Bit 0 was 0
    EXPECT_TRUE(cpu.Z);   // Result is 0
    EXPECT_FALSE(cpu.N);  // Bit 7 is always 0 in LSR
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502LogicalShiftRightTest, LSRAccumulatorShiftsBit0IntoCarrySetsZeroFlag) {
    cpu.PC = 0x8000;
    cpu.A = 0x01; // Bit 0 set
    cpu.C = false;

    mem[0x8000] = CPU::INS_LSR_A;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8001);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_EQ(cpu.A, 0x00); // 0x01 >> 1 = 0x00
    EXPECT_TRUE(cpu.C);   // Bit 0 was 1 -> pushed into carry
    EXPECT_TRUE(cpu.Z);   // Result is 0
    EXPECT_FALSE(cpu.N);  // Bit 7 is 0
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502LogicalShiftRightTest, LSRAccumulatorShiftsEvenValueLeavesCarryClear) {
    cpu.PC = 0x8000;
    cpu.A = 0x04;
    cpu.C = true; // Pre-existing carry must be overwritten

    mem[0x8000] = CPU::INS_LSR_A;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8001);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_EQ(cpu.A, 0x02); // 0x04 >> 1 = 0x02
    EXPECT_FALSE(cpu.C);  // Bit 0 was 0
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);  // Bit 7 is 0
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502LogicalShiftRightTest, LSRAccumulatorHighBitBecomesZeroAndNegativeFlagIsAlwaysFalse) {
    cpu.PC = 0x8000;
    cpu.A = 0x80; // Negative input (bit 7 set)
    cpu.C = true;

    mem[0x8000] = CPU::INS_LSR_A;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8001);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_EQ(cpu.A, 0x40); // 0x80 >> 1 = 0x40
    EXPECT_FALSE(cpu.C);  // Bit 0 was 0
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);  // LSR always clears N flag
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502LogicalShiftRightTest, LSRAccumulatorShiftsBothEndsCorrectly) {
    cpu.PC = 0x8000;
    cpu.A = 0x81; // 0b10000001
    cpu.C = false;

    mem[0x8000] = CPU::INS_LSR_A;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8001);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_EQ(cpu.A, 0x40); // 0b01000000
    EXPECT_TRUE(cpu.C);   // Bit 0 was 1
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);  // Bit 7 is 0
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

// ---------------------------------------------------------------------------
// Memory Addressing Modes (Read-Modify-Write instructions)
// ---------------------------------------------------------------------------

TEST_F(M6502LogicalShiftRightTest, LSRZeroPageShiftsMemoryLeavesAccumulatorUntouched) {
    cpu.PC = 0x8000;
    cpu.A = 0xAA; // Accumulator must not be altered by memory operations
    cpu.C = false;

    mem[0x8000] = CPU::INS_LSR_ZP;
    mem[0x8001] = 0x42;
    mem[0x0042] = 0x0A; // 0x0A >> 1 = 0x05

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 5); // 6502 LSR ZeroPage takes 5 cycles

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 5);
    EXPECT_EQ(mem[0x0042], 0x05);
    EXPECT_EQ(cpu.A, 0xAA); // Verifies A is untouched
    EXPECT_FALSE(cpu.C);  // Bit 0 of 0x0A was 0
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502LogicalShiftRightTest, LSRZeroPageMemoryZeroResultSetsZeroFlagIndependentlyOfAccumulator) {
    cpu.PC = 0x8000;
    cpu.A = 0xFF; // A != 0, tests that Z flag is determined by the memory result
    cpu.C = false;

    mem[0x8000] = CPU::INS_LSR_ZP;
    mem[0x8001] = 0x10;
    mem[0x0010] = 0x01; // 0x01 >> 1 = 0x00, Carry=1

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 5);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 5);
    EXPECT_EQ(mem[0x0010], 0x00);
    EXPECT_EQ(cpu.A, 0xFF);
    EXPECT_TRUE(cpu.C);
    EXPECT_TRUE(cpu.Z);   // Reflects memory shift result
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502LogicalShiftRightTest, LSRZeroPageXShiftsMemoryCorrectlyTakes6Cycles) {
    cpu.PC = 0x8000;
    cpu.A = 0x55;
    cpu.X = 0x05;
    cpu.C = false;

    mem[0x8000] = CPU::INS_LSR_ZP_X;
    mem[0x8001] = 0x20;
    mem[0x0025] = 0x07; // 0x07 >> 1 = 0x03, Carry=1

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 6); // 6502 LSR ZP,X takes 6 cycles

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 6);
    EXPECT_EQ(mem[0x0025], 0x03);
    EXPECT_EQ(cpu.A, 0x55);
    EXPECT_TRUE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502LogicalShiftRightTest, LSRZeroPageXWrapsWithinZeroPage) {
    cpu.PC = 0x8000;
    cpu.X = 0xFF;
    cpu.C = false;

    mem[0x8000] = CPU::INS_LSR_ZP_X;
    mem[0x8001] = 0x80;
    mem[0x007F] = 0x10; // 0x80 + 0xFF wraps to 0x007F. 0x10 >> 1 = 0x08

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 6);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 6);
    EXPECT_EQ(mem[0x007F], 0x08);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502LogicalShiftRightTest, LSRAbsoluteShiftsMemoryCorrectlyTakes6Cycles) {
    cpu.PC = 0x8000;
    cpu.C = false;

    mem[0x8000] = CPU::INS_LSR_ABS;
    mem[0x8001] = 0x34;
    mem[0x8002] = 0x12;
    mem[0x1234] = 0x11; // 0x11 >> 1 = 0x08, Carry=1

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 6); // 6502 LSR Absolute takes 6 cycles

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 6);
    EXPECT_EQ(mem[0x1234], 0x08);
    EXPECT_TRUE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502LogicalShiftRightTest, LSRAbsoluteXAlwaysTakes7CyclesEvenWithoutPageCrossing) {
    cpu.PC = 0x8000;
    cpu.X = 0x05;
    cpu.C = false;

    mem[0x8000] = CPU::INS_LSR_ABS_X;
    mem[0x8001] = 0x10;
    mem[0x8002] = 0x20;
    mem[0x2015] = 0x02; // 0x02 >> 1 = 0x01

    const CPU cpuCopy = cpu;
    // RMW instructions on 6502 always take 7 cycles for ABS,X regardless of page crossing
    s32 cyclesUsed = cpu.Execute(mem, 7);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 7);
    EXPECT_EQ(mem[0x2015], 0x01);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502LogicalShiftRightTest, LSRAbsoluteXWithPageCrossingTakes7Cycles) {
    cpu.PC = 0x8000;
    cpu.X = 0x05;
    cpu.C = false;

    mem[0x8000] = CPU::INS_LSR_ABS_X;
    mem[0x8001] = 0xFD;
    mem[0x8002] = 0x20;
    mem[0x2102] = 0x03; // Crosses page boundary to 0x2102. 0x03 >> 1 = 0x01, Carry=1

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 7);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 7);
    EXPECT_EQ(mem[0x2102], 0x01);
    EXPECT_TRUE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}