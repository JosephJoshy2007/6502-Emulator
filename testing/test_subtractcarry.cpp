#include <gtest/gtest.h>
#include "m6502.h"

class M6502SubtractCarryTest : public ::testing::Test {
protected:
    Memory mem;
    CPU cpu;

    virtual void SetUp() override {
        cpu.Reset(mem);
    }

    virtual void TearDown() override {
    }
};

void VerifyUnchangedNonArithmeticFlags(const CPU& current, const CPU& original) {
    EXPECT_EQ(current.B, original.B);
    EXPECT_EQ(current.D, original.D);
    EXPECT_EQ(current.I, original.I);
}

TEST_F(M6502SubtractCarryTest, SBCSubtractsZeroMinusZeroWithCarrySet) {
    cpu.PC = 0x8000;
    cpu.A = 0x00;
    cpu.C = true; // C=1: No borrow

    mem[0x8000] = CPU::INS_SBC_ABS;
    mem[0x8001] = 0x00;
    mem[0x8002] = 0x70;
    mem[0x7000] = 0x00;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_TRUE(cpu.C);   // No borrow occurred
    EXPECT_TRUE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_FALSE(cpu.V);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502SubtractCarryTest, SBCProducesZeroAndMaintainsCarryAndZeroFlag) {
    cpu.PC = 0x8000;
    cpu.A = 0x80;
    cpu.C = true; // C=1: No borrow

    mem[0x8000] = CPU::INS_SBC_ABS;
    mem[0x8001] = 0x00;
    mem[0x8002] = 0x70;
    mem[0x7000] = 0x80; // 0x80 - 0x80 = 0x00

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_TRUE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_TRUE(cpu.C);   // Result >= 0, carry kept
    EXPECT_FALSE(cpu.V);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502SubtractCarryTest, SBCBorrowGeneratesNegativeResult) {
    cpu.PC = 0x8000;
    cpu.A = 0x00;
    cpu.C = true;

    mem[0x8000] = CPU::INS_SBC_ABS;
    mem[0x8001] = 0x00;
    mem[0x8002] = 0x70;
    mem[0x7000] = 0x01; // 0x00 - 0x01 = 0xFF (-1)

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_EQ(cpu.A, 0xFF);
    EXPECT_FALSE(cpu.C);  // Borrow required -> C cleared
    EXPECT_FALSE(cpu.Z);
    EXPECT_TRUE(cpu.N);   // Bit 7 is set
    EXPECT_FALSE(cpu.V);  // 0 - 1 = -1 (fits in signed char, no overflow)
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502SubtractCarryTest, SBCPositiveMinusNegativeCausesSignedOverflow) {
    cpu.PC = 0x8000;
    cpu.A = 0x50; // +80
    cpu.C = true;

    mem[0x8000] = CPU::INS_SBC_ABS;
    mem[0x8001] = 0x00;
    mem[0x8002] = 0x70;
    mem[0x7000] = 0xB0; // -80 in two's complement

    // +80 - (-80) = +160 -> wraps to 0xA0 (-96) -> Overflow!
    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_EQ(cpu.A, 0xA0);
    EXPECT_FALSE(cpu.C);  // Unsigned: 0x50 < 0xB0 -> borrow occurred
    EXPECT_TRUE(cpu.V);   // Positive - Negative yielded Negative
    EXPECT_TRUE(cpu.N);
    EXPECT_FALSE(cpu.Z);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502SubtractCarryTest, SBCPositiveMinusNegativeMaxFitNoOverflow) {
    cpu.PC = 0x8000;
    cpu.A = 0x50; // +80
    cpu.C = true;

    mem[0x8000] = CPU::INS_SBC_ABS;
    mem[0x8001] = 0x00;
    mem[0x8002] = 0x70;
    mem[0x7000] = 0xD1; // -47 in two's complement

    // +80 - (-47) = +127 (0x7F) -> Fits in signed range
    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_EQ(cpu.A, 0x7F);
    EXPECT_FALSE(cpu.C);  // Unsigned: 0x50 < 0xD1 -> borrow occurred
    EXPECT_FALSE(cpu.V);
    EXPECT_FALSE(cpu.N);
    EXPECT_FALSE(cpu.Z);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502SubtractCarryTest, SBCNegativeMinusPositiveCausesSignedOverflow) {
    cpu.PC = 0x8000;
    cpu.A = 0x80; // -128
    cpu.C = true;

    mem[0x8000] = CPU::INS_SBC_ABS;
    mem[0x8001] = 0x00;
    mem[0x8002] = 0x70;
    mem[0x7000] = 0x01; // +1

    // -128 - 1 = -129 -> underflows to 0x7F (+127) -> Overflow!
    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_EQ(cpu.A, 0x7F);
    EXPECT_TRUE(cpu.C);   // Unsigned: 0x80 >= 0x01 -> No borrow
    EXPECT_TRUE(cpu.V);   // Negative - Positive yielded Positive
    EXPECT_FALSE(cpu.N);
    EXPECT_FALSE(cpu.Z);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502SubtractCarryTest, SBCSameSignSubtractionNeverOverflows) {
    cpu.PC = 0x8000;
    cpu.A = 0x50; // +80
    cpu.C = true;

    mem[0x8000] = CPU::INS_SBC_ABS;
    mem[0x8001] = 0x00;
    mem[0x8002] = 0x70;
    mem[0x7000] = 0x70; // +112

    // Pos - Pos cannot cause signed overflow
    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_EQ(cpu.A, 0xE0); // 80 - 112 = -32 (0xE0)
    EXPECT_FALSE(cpu.C);  // Unsigned borrow occurred
    EXPECT_FALSE(cpu.V);
    EXPECT_TRUE(cpu.N);
    EXPECT_FALSE(cpu.Z);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502SubtractCarryTest, SBCIncomingBorrowSubtractsExtraOne) {
    cpu.PC = 0x8000;
    cpu.A = 0x00;
    cpu.C = false; // C=0: Borrow active (subtracts extra 1)

    mem[0x8000] = CPU::INS_SBC_ABS;
    mem[0x8001] = 0x00;
    mem[0x8002] = 0x70;
    mem[0x7000] = 0x00; // 0 - 0 - (1 - 0) = -1 (0xFF)

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_EQ(cpu.A, 0xFF);
    EXPECT_FALSE(cpu.C);  // Borrow required
    EXPECT_FALSE(cpu.Z);
    EXPECT_TRUE(cpu.N);
    EXPECT_FALSE(cpu.V);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502SubtractCarryTest, SBCIncomingBorrowTriggersSignedOverflow) {
    cpu.PC = 0x8000;
    cpu.A = 0x80; // -128
    cpu.C = false; // C=0: extra -1

    mem[0x8000] = CPU::INS_SBC_ABS;
    mem[0x8001] = 0x00;
    mem[0x8002] = 0x70;
    mem[0x7000] = 0x00; // -128 - 0 - 1 = -129 -> 0x7F (+127)

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_EQ(cpu.A, 0x7F);
    EXPECT_TRUE(cpu.C);   // 0x80 + 0xFF + 0 = 0x17F >= 0x100 -> C=1
    EXPECT_TRUE(cpu.V);   // Overflow into positive
    EXPECT_FALSE(cpu.N);
    EXPECT_FALSE(cpu.Z);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502SubtractCarryTest, SBCImmediateSubtractsValueCorrectly) {
    cpu.PC = 0x8000;
    cpu.A = 0x35;
    cpu.C = true;

    mem[0x8000] = CPU::INS_SBC_IMM;
    mem[0x8001] = 0x15;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_EQ(cpu.A, 0x20);
    EXPECT_TRUE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_FALSE(cpu.V);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502SubtractCarryTest, SBCZeroPageSubtractsValueCorrectly) {
    cpu.PC = 0x8000;
    cpu.A = 0x36;
    cpu.C = true;

    mem[0x8000] = CPU::INS_SBC_ZP;
    mem[0x8001] = 0x42;
    mem[0x0042] = 0x25; // 0x36 - 0x25 = 0x11

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 3);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 3);
    EXPECT_EQ(cpu.A, 0x11);
    EXPECT_TRUE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_FALSE(cpu.V);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502SubtractCarryTest, SBCZeroPageXSubtractsValueCorrectly) {
    cpu.PC = 0x8000;
    cpu.A = 0x17;
    cpu.X = 0x08;
    cpu.C = true;

    mem[0x8000] = CPU::INS_SBC_ZP_X;
    mem[0x8001] = 0x20;
    mem[0x0028] = 0x12; // 0x17 - 0x12 = 0x05

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_EQ(cpu.A, 0x05);
    EXPECT_TRUE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_FALSE(cpu.V);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502SubtractCarryTest, SBCZeroPageXWrapsAroundZeroPage) {
    cpu.PC = 0x8000;
    cpu.A = 0x0C;
    cpu.X = 0xFF;
    cpu.C = true;

    mem[0x8000] = CPU::INS_SBC_ZP_X;
    mem[0x8001] = 0x80;
    mem[0x007F] = 0x0A; // 0x0C - 0x0A = 0x02

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_EQ(cpu.A, 0x02);
    EXPECT_TRUE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_FALSE(cpu.V);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502SubtractCarryTest, SBCAbsoluteSubtractsValueCorrectly) {
    cpu.PC = 0x8000;
    cpu.A = 0x41;
    cpu.C = true;

    mem[0x8000] = CPU::INS_SBC_ABS;
    mem[0x8001] = 0xCD;
    mem[0x8002] = 0xAB;
    mem[0xABCD] = 0x11; // 0x41 - 0x11 = 0x30

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_EQ(cpu.A, 0x30);
    EXPECT_TRUE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_FALSE(cpu.V);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502SubtractCarryTest, SBCAbsoluteXSubtractsValueSamePageTakes4Cycles) {
    cpu.PC = 0x8000;
    cpu.A = 0x32;
    cpu.X = 0x05;
    cpu.C = true;

    mem[0x8000] = CPU::INS_SBC_ABS_X;
    mem[0x8001] = 0x10;
    mem[0x8002] = 0x20;
    mem[0x2015] = 0x22; // 0x32 - 0x22 = 0x10

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_EQ(cpu.A, 0x10);
    EXPECT_TRUE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_FALSE(cpu.V);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502SubtractCarryTest, SBCAbsoluteXSubtractsValuePageCrossingTakes5Cycles) {
    cpu.PC = 0x8000;
    cpu.A = 0x32;
    cpu.X = 0x05;
    cpu.C = true;

    mem[0x8000] = CPU::INS_SBC_ABS_X;
    mem[0x8001] = 0xFD;
    mem[0x8002] = 0x20;
    mem[0x2102] = 0x22;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 5);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 5);
    EXPECT_EQ(cpu.A, 0x10);
    EXPECT_TRUE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_FALSE(cpu.V);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502SubtractCarryTest, SBCAbsoluteYSubtractsValueSamePageTakes4Cycles) {
    cpu.PC = 0x8000;
    cpu.A = 0x1A;
    cpu.Y = 0x08;
    cpu.C = true;

    mem[0x8000] = CPU::INS_SBC_ABS_Y;
    mem[0x8001] = 0x20;
    mem[0x8002] = 0x30;
    mem[0x3028] = 0x06; // 0x1A - 0x06 = 0x14

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_EQ(cpu.A, 0x14);
    EXPECT_TRUE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_FALSE(cpu.V);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502SubtractCarryTest, SBCAbsoluteYSubtractsValuePageCrossingTakes5Cycles) {
    cpu.PC = 0x8000;
    cpu.A = 0x1A;
    cpu.Y = 0x08;
    cpu.C = true;

    mem[0x8000] = CPU::INS_SBC_ABS_Y;
    mem[0x8001] = 0xFA;
    mem[0x8002] = 0x30;
    mem[0x3102] = 0x06;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 5);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 5);
    EXPECT_EQ(cpu.A, 0x14);
    EXPECT_TRUE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_FALSE(cpu.V);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502SubtractCarryTest, SBCIndirectXSubtractsValueCorrectlyTakes6Cycles) {
    cpu.PC = 0x8000;
    cpu.A = 0x0A;
    cpu.X = 0x04;
    cpu.C = true;

    mem[0x8000] = CPU::INS_SBC_IND_X;
    mem[0x8001] = 0x20;
    mem[0x0024] = 0x00;
    mem[0x0025] = 0x40;
    mem[0x4000] = 0x07; // 0x0A - 0x07 = 0x03

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 6);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 6);
    EXPECT_EQ(cpu.A, 0x03);
    EXPECT_TRUE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_FALSE(cpu.V);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502SubtractCarryTest, SBCIndirectXWrapsAroundZeroPageDuringPointerFetch) {
    cpu.PC = 0x8000;
    cpu.A = 0x15;
    cpu.X = 0x05;
    cpu.C = true;

    mem[0x8000] = CPU::INS_SBC_IND_X;
    mem[0x8001] = 0xFB;
    mem[0x0000] = 0x80;
    mem[0x0001] = 0x50;
    mem[0x5080] = 0x10; // 0x15 - 0x10 = 0x05

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 6);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 6);
    EXPECT_EQ(cpu.A, 0x05);
    EXPECT_TRUE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_FALSE(cpu.V);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502SubtractCarryTest, SBCIndirectXPointerBoundaryWrapsWithinZeroPage) {
    cpu.PC = 0x8000;
    cpu.A = 0x0A;
    cpu.X = 0x00;
    cpu.C = true;

    mem[0x8000] = CPU::INS_SBC_IND_X;
    mem[0x8001] = 0xFF;
    mem[0x00FF] = 0x34;
    mem[0x0000] = 0x12;
    mem[0x1234] = 0x08; // 0x0A - 0x08 = 0x02

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 6);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 6);
    EXPECT_EQ(cpu.A, 0x02);
    EXPECT_TRUE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_FALSE(cpu.V);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502SubtractCarryTest, SBCIndirectYSamePageTakes5Cycles) {
    cpu.PC = 0x8000;
    cpu.A = 0x0A;
    cpu.Y = 0x04;
    cpu.C = true;

    mem[0x8000] = CPU::INS_SBC_IND_Y;
    mem[0x8001] = 0x40;
    mem[0x0040] = 0x10;
    mem[0x0041] = 0x60;
    mem[0x6014] = 0x09; // 0x0A - 0x09 = 0x01

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 5);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 5);
    EXPECT_EQ(cpu.A, 0x01);
    EXPECT_TRUE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_FALSE(cpu.V);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502SubtractCarryTest, SBCIndirectYPageCrossingTakes6Cycles) {
    cpu.PC = 0x8000;
    cpu.A = 0x0A;
    cpu.Y = 0x05;
    cpu.C = true;

    mem[0x8000] = CPU::INS_SBC_IND_Y;
    mem[0x8001] = 0x40;
    mem[0x0040] = 0xFD;
    mem[0x0041] = 0x60;
    mem[0x6102] = 0x08; // 0x0A - 0x08 = 0x02

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 6);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 6);
    EXPECT_EQ(cpu.A, 0x02);
    EXPECT_TRUE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_FALSE(cpu.V);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502SubtractCarryTest, SBCIndirectYPointerBoundaryWrapsWithinZeroPage) {
    cpu.PC = 0x8000;
    cpu.A = 0x0A;
    cpu.Y = 0x01;
    cpu.C = true;

    mem[0x8000] = CPU::INS_SBC_IND_Y;
    mem[0x8001] = 0xFF;
    mem[0x00FF] = 0x00;
    mem[0x0000] = 0x70;
    mem[0x7001] = 0x05; // 0x0A - 0x05 = 0x05

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 5);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 5);
    EXPECT_EQ(cpu.A, 0x05);
    EXPECT_TRUE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_FALSE(cpu.V);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}