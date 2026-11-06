#include <gtest/gtest.h>
#include "m6502.h"

class M6502RotateTest : public ::testing::Test {
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
    EXPECT_EQ(current.V, original.V);
}

// ===========================================================================
// ROL (Rotate Left) Tests
// ===========================================================================

TEST_F(M6502RotateTest, ROLAccumulatorZeroWithCarryClearProducesZero) {
    cpu.PC = 0x8000;
    cpu.A = 0x00;
    cpu.C = false;

    mem[0x8000] = CPU::INS_ROL_A;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8001);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_FALSE(cpu.C);
    EXPECT_TRUE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502RotateTest, ROLAccumulatorRotatesIncomingCarryIntoBit0) {
    cpu.PC = 0x8000;
    cpu.A = 0x00;
    cpu.C = true;

    mem[0x8000] = CPU::INS_ROL_A;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8001);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_EQ(cpu.A, 0x01);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502RotateTest, ROLAccumulatorShiftsBit7IntoCarrySetsZeroFlag) {
    cpu.PC = 0x8000;
    cpu.A = 0x80;
    cpu.C = false;

    mem[0x8000] = CPU::INS_ROL_A;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8001);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_TRUE(cpu.C);
    EXPECT_TRUE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502RotateTest, ROLAccumulatorFullRotationWithIncomingAndOutgoingCarry) {
    cpu.PC = 0x8000;
    cpu.A = 0xC0;
    cpu.C = true;

    mem[0x8000] = CPU::INS_ROL_A;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8001);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_EQ(cpu.A, 0x81);
    EXPECT_TRUE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_TRUE(cpu.N);
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502RotateTest, ROLZeroPageShiftsMemoryLeavesAccumulatorUntouched) {
    cpu.PC = 0x8000;
    cpu.A = 0x55;
    cpu.C = true;

    mem[0x8000] = CPU::INS_ROL_ZP;
    mem[0x8001] = 0x42;
    mem[0x0042] = 0x02;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 5);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 5);
    EXPECT_EQ(mem[0x0042], 0x05);
    EXPECT_EQ(cpu.A, 0x55);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502RotateTest, ROLZeroPageMemoryZeroResultSetsZeroFlagIndependentlyOfAccumulator) {
    cpu.PC = 0x8000;
    cpu.A = 0xFF;
    cpu.C = false;

    mem[0x8000] = CPU::INS_ROL_ZP;
    mem[0x8001] = 0x10;
    mem[0x0010] = 0x80;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 5);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 5);
    EXPECT_EQ(mem[0x0010], 0x00);
    EXPECT_EQ(cpu.A, 0xFF);
    EXPECT_TRUE(cpu.C);
    EXPECT_TRUE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502RotateTest, ROLZeroPageXShiftsMemoryCorrectlyTakes6Cycles) {
    cpu.PC = 0x8000;
    cpu.A = 0x10;
    cpu.X = 0x05;
    cpu.C = false;

    mem[0x8000] = CPU::INS_ROL_ZP_X;
    mem[0x8001] = 0x20;
    mem[0x0025] = 0x04;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 6);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 6);
    EXPECT_EQ(mem[0x0025], 0x08);
    EXPECT_EQ(cpu.A, 0x10);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502RotateTest, ROLZeroPageXWrapsWithinZeroPage) {
    cpu.PC = 0x8000;
    cpu.X = 0xFF;
    cpu.C = true;

    mem[0x8000] = CPU::INS_ROL_ZP_X;
    mem[0x8001] = 0x80;
    mem[0x007F] = 0x40;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 6);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 6);
    EXPECT_EQ(mem[0x007F], 0x81);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_TRUE(cpu.N);
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502RotateTest, ROLAbsoluteShiftsMemoryCorrectlyTakes6Cycles) {
    cpu.PC = 0x8000;
    cpu.C = true;

    mem[0x8000] = CPU::INS_ROL_ABS;
    mem[0x8001] = 0x34;
    mem[0x8002] = 0x12;
    mem[0x1234] = 0x00;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 6);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 6);
    EXPECT_EQ(mem[0x1234], 0x01);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502RotateTest, ROLAbsoluteXAlwaysTakes7CyclesEvenWithoutPageCrossing) {
    cpu.PC = 0x8000;
    cpu.X = 0x05;
    cpu.C = false;

    mem[0x8000] = CPU::INS_ROL_ABS_X;
    mem[0x8001] = 0x10;
    mem[0x8002] = 0x20;
    mem[0x2015] = 0x03;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 7);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 7);
    EXPECT_EQ(mem[0x2015], 0x06);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502RotateTest, ROLAbsoluteXWithPageCrossingTakes7Cycles) {
    cpu.PC = 0x8000;
    cpu.X = 0x05;
    cpu.C = true;

    mem[0x8000] = CPU::INS_ROL_ABS_X;
    mem[0x8001] = 0xFD;
    mem[0x8002] = 0x20;
    mem[0x2102] = 0x82;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 7);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 7);
    EXPECT_EQ(mem[0x2102], 0x05);
    EXPECT_TRUE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

// ===========================================================================
// ROR (Rotate Right) Tests
// ===========================================================================

TEST_F(M6502RotateTest, RORAccumulatorZeroWithCarryClearProducesZero) {
    cpu.PC = 0x8000;
    cpu.A = 0x00;
    cpu.C = false;

    mem[0x8000] = CPU::INS_ROR_A;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8001);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_FALSE(cpu.C);
    EXPECT_TRUE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502RotateTest, RORAccumulatorRotatesIncomingCarryIntoBit7) {
    cpu.PC = 0x8000;
    cpu.A = 0x00;
    cpu.C = true;

    mem[0x8000] = CPU::INS_ROR_A;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8001);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_EQ(cpu.A, 0x80);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_TRUE(cpu.N);
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502RotateTest, RORAccumulatorShiftsBit0IntoCarryWithoutIncomingCarry) {
    cpu.PC = 0x8000;
    cpu.A = 0x01;
    cpu.C = false;

    mem[0x8000] = CPU::INS_ROR_A;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8001);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_TRUE(cpu.C);
    EXPECT_TRUE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502RotateTest, RORAccumulatorFullRotationWithIncomingAndOutgoingCarry) {
    cpu.PC = 0x8000;
    cpu.A = 0x03;
    cpu.C = true;

    mem[0x8000] = CPU::INS_ROR_A;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8001);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_EQ(cpu.A, 0x81);
    EXPECT_TRUE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_TRUE(cpu.N);
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502RotateTest, RORAccumulatorNegativeInputWithoutIncomingCarryClearsBit7) {
    cpu.PC = 0x8000;
    cpu.A = 0x80;
    cpu.C = false;

    mem[0x8000] = CPU::INS_ROR_A;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8001);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_EQ(cpu.A, 0x40);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502RotateTest, RORZeroPageShiftsMemoryLeavesAccumulatorUntouched) {
    cpu.PC = 0x8000;
    cpu.A = 0x55;
    cpu.C = true;

    mem[0x8000] = CPU::INS_ROR_ZP;
    mem[0x8001] = 0x42;
    mem[0x0042] = 0x04;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 5);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 5);
    EXPECT_EQ(mem[0x0042], 0x82);
    EXPECT_EQ(cpu.A, 0x55);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_TRUE(cpu.N);
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502RotateTest, RORZeroPageMemoryZeroResultSetsZeroFlagIndependentlyOfAccumulator) {
    cpu.PC = 0x8000;
    cpu.A = 0xFF;
    cpu.C = false;

    mem[0x8000] = CPU::INS_ROR_ZP;
    mem[0x8001] = 0x10;
    mem[0x0010] = 0x01;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 5);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 5);
    EXPECT_EQ(mem[0x0010], 0x00);
    EXPECT_EQ(cpu.A, 0xFF);
    EXPECT_TRUE(cpu.C);
    EXPECT_TRUE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502RotateTest, RORZeroPageXShiftsMemoryCorrectlyTakes6Cycles) {
    cpu.PC = 0x8000;
    cpu.A = 0x10;
    cpu.X = 0x05;
    cpu.C = false;

    mem[0x8000] = CPU::INS_ROR_ZP_X;
    mem[0x8001] = 0x20;
    mem[0x0025] = 0x0A;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 6);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 6);
    EXPECT_EQ(mem[0x0025], 0x05);
    EXPECT_EQ(cpu.A, 0x10);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502RotateTest, RORZeroPageXWrapsWithinZeroPage) {
    cpu.PC = 0x8000;
    cpu.X = 0xFF;
    cpu.C = true;

    mem[0x8000] = CPU::INS_ROR_ZP_X;
    mem[0x8001] = 0x80;
    mem[0x007F] = 0x02;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 6);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 6);
    EXPECT_EQ(mem[0x007F], 0x81);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_TRUE(cpu.N);
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502RotateTest, RORAbsoluteShiftsMemoryCorrectlyTakes6Cycles) {
    cpu.PC = 0x8000;
    cpu.C = true;

    mem[0x8000] = CPU::INS_ROR_ABS;
    mem[0x8001] = 0x34;
    mem[0x8002] = 0x12;
    mem[0x1234] = 0x00;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 6);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 6);
    EXPECT_EQ(mem[0x1234], 0x80);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_TRUE(cpu.N);
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502RotateTest, RORAbsoluteXAlwaysTakes7CyclesEvenWithoutPageCrossing) {
    cpu.PC = 0x8000;
    cpu.X = 0x05;
    cpu.C = false;

    mem[0x8000] = CPU::INS_ROR_ABS_X;
    mem[0x8001] = 0x10;
    mem[0x8002] = 0x20;
    mem[0x2015] = 0x06;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 7);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 7);
    EXPECT_EQ(mem[0x2015], 0x03);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}

TEST_F(M6502RotateTest, RORAbsoluteXWithPageCrossingTakes7Cycles) {
    cpu.PC = 0x8000;
    cpu.X = 0x05;
    cpu.C = true;

    mem[0x8000] = CPU::INS_ROR_ABS_X;
    mem[0x8001] = 0xFD;
    mem[0x8002] = 0x20;
    mem[0x2102] = 0x07;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 7);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 7);
    EXPECT_EQ(mem[0x2102], 0x83);
    EXPECT_TRUE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_TRUE(cpu.N);
    VerifyUnchangedNonShiftFlags(cpu, cpuCopy);
}