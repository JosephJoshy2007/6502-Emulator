#include <gtest/gtest.h>
#include "m6502.h"

class M6502ShiftLeftTest : public ::testing::Test {
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
    EXPECT_EQ(current.V, original.V); // ASL does not touch overflow
}

// ---------------------------------------------------------------------------
// Flag & Boundary Behavior (tested via ASL Accumulator)
// ---------------------------------------------------------------------------

TEST_F(M6502ShiftLeftTest, ASLAccumulatorShiftsZeroYieldsZeroSetsZeroFlag) {
    cpu.PC = 0x8000;
    cpu.A = 0x00;
    cpu.C = true; // Pre-existing carry should be cleared by 0 shifted out

    mem[0x8000] = CPU::INS_ASL_A;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8001);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_FALSE(cpu.C);  // Bit 7 was 0
    EXPECT_TRUE(cpu.Z);   // Result is 0
    EXPECT_FALSE(cpu.N);  // Bit 7 of result is 0
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502ShiftLeftTest, ASLAccumulatorShiftsBit7IntoCarrySetsZeroFlag) {
    cpu.PC = 0x8000;
    cpu.A = 0x80;
    cpu.C = false;

    mem[0x8000] = CPU::INS_ASL_A;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8001);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_EQ(cpu.A, 0x00); // 0x80 << 1 = 0x00 (truncated to 8 bits)
    EXPECT_TRUE(cpu.C);   // Bit 7 was 1 -> pushed into carry
    EXPECT_TRUE(cpu.Z);   // Lower 8-bit result became 0
    EXPECT_FALSE(cpu.N);  // Bit 7 of 0x00 is 0
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502ShiftLeftTest, ASLAccumulatorShiftsBitIntoSignBitSetsNegativeFlag) {
    cpu.PC = 0x8000;
    cpu.A = 0x40; // Bit 6 set
    cpu.C = true;

    mem[0x8000] = CPU::INS_ASL_A;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8001);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_EQ(cpu.A, 0x80); // 0x40 << 1 = 0x80
    EXPECT_FALSE(cpu.C);  // Bit 7 was 0
    EXPECT_FALSE(cpu.Z);
    EXPECT_TRUE(cpu.N);   // Bit 7 of 0x80 is 1
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502ShiftLeftTest, ASLAccumulatorShiftsBit7AndKeepsNegativeFlag) {
    cpu.PC = 0x8000;
    cpu.A = 0xC0; // 0b11000000 -> bit 7 goes to C, bit 6 goes to bit 7
    cpu.C = false;

    mem[0x8000] = CPU::INS_ASL_A;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8001);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_EQ(cpu.A, 0x80);
    EXPECT_TRUE(cpu.C);   // Old bit 7 was 1
    EXPECT_FALSE(cpu.Z);
    EXPECT_TRUE(cpu.N);   // New bit 7 is 1
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

// ---------------------------------------------------------------------------
// Memory Addressing Modes (Read-Modify-Write instructions)
// ---------------------------------------------------------------------------

TEST_F(M6502ShiftLeftTest, ASLZeroPageShiftsMemoryLeavesAccumulatorUntouched) {
    cpu.PC = 0x8000;
    cpu.A = 0xAA; // Accumulator should remain completely unmodified
    cpu.C = false;

    mem[0x8000] = CPU::INS_ASL_ZP;
    mem[0x8001] = 0x42;
    mem[0x0042] = 0x05; // 0x05 << 1 = 0x0A

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 5); // 6502 ASL ZeroPage takes 5 cycles

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 5);
    EXPECT_EQ(mem[0x0042], 0x0A);
    EXPECT_EQ(cpu.A, 0xAA); // Verifies A is untouched
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502ShiftLeftTest, ASLZeroPageMemoryZeroResultSetsZeroFlagIndependentlyOfAccumulator) {
    cpu.PC = 0x8000;
    cpu.A = 0xFF; // A != 0, tests that Z flag is determined by the memory shift
    cpu.C = false;

    mem[0x8000] = CPU::INS_ASL_ZP;
    mem[0x8001] = 0x10;
    mem[0x0010] = 0x80; // 0x80 << 1 = 0x00, Carry=1

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 5);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 5);
    EXPECT_EQ(mem[0x0010], 0x00);
    EXPECT_EQ(cpu.A, 0xFF);
    EXPECT_TRUE(cpu.C);
    EXPECT_TRUE(cpu.Z);   // Must reflect memory result (0), not A (0xFF)
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502ShiftLeftTest, ASLZeroPageXShiftsMemoryCorrectlyTakes6Cycles) {
    cpu.PC = 0x8000;
    cpu.A = 0x55;
    cpu.X = 0x05;
    cpu.C = false;

    mem[0x8000] = CPU::INS_ASL_ZP_X;
    mem[0x8001] = 0x20;
    mem[0x0025] = 0x03; // 0x03 << 1 = 0x06

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 6); // 6502 ASL ZP,X takes 6 cycles

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 6);
    EXPECT_EQ(mem[0x0025], 0x06);
    EXPECT_EQ(cpu.A, 0x55);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502ShiftLeftTest, ASLZeroPageXWrapsWithinZeroPage) {
    cpu.PC = 0x8000;
    cpu.X = 0xFF;
    cpu.C = false;

    mem[0x8000] = CPU::INS_ASL_ZP_X;
    mem[0x8001] = 0x80;
    mem[0x007F] = 0x40; // 0x80 + 0xFF wraps to 0x007F. 0x40 << 1 = 0x80

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 6);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 6);
    EXPECT_EQ(mem[0x007F], 0x80);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_TRUE(cpu.N);
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502ShiftLeftTest, ASLAbsoluteShiftsMemoryCorrectlyTakes6Cycles) {
    cpu.PC = 0x8000;
    cpu.C = false;

    mem[0x8000] = CPU::INS_ASL_ABS;
    mem[0x8001] = 0x34;
    mem[0x8002] = 0x12;
    mem[0x1234] = 0x08; // 0x08 << 1 = 0x10

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 6); // 6502 ASL Absolute takes 6 cycles

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 6);
    EXPECT_EQ(mem[0x1234], 0x10);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502ShiftLeftTest, ASLAbsoluteXAlwaysTakes7CyclesEvenWithoutPageCrossing) {
    cpu.PC = 0x8000;
    cpu.X = 0x05;
    cpu.C = false;

    mem[0x8000] = CPU::INS_ASL_ABS_X;
    mem[0x8001] = 0x10;
    mem[0x8002] = 0x20;
    mem[0x2015] = 0x01; // 0x01 << 1 = 0x02

    const CPU cpuCopy = cpu;
    // Note: RMW instructions on 6502 always take 7 cycles for ABS,X regardless of page crossing
    s32 cyclesUsed = cpu.Execute(mem, 7);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 7);
    EXPECT_EQ(mem[0x2015], 0x02);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502ShiftLeftTest, ASLAbsoluteXWithPageCrossingTakes7Cycles) {
    cpu.PC = 0x8000;
    cpu.X = 0x05;
    cpu.C = false;

    mem[0x8000] = CPU::INS_ASL_ABS_X;
    mem[0x8001] = 0xFD;
    mem[0x8002] = 0x20;
    mem[0x2102] = 0x81; // Crosses page boundary to 0x2102. 0x81 << 1 = 0x02, Carry=1

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 7);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 7);
    EXPECT_EQ(mem[0x2102], 0x02);
    EXPECT_TRUE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}