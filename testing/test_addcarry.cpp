#include <gtest/gtest.h>
#include "m6502.h"

class M6502AddCarryTest : public ::testing::Test {
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


TEST_F(M6502AddCarryTest, ADCAddsZeroPlusZero) {
    cpu.PC = 0x8000;
    cpu.A = 0x00;
    cpu.C = false;

    mem[0x8000] = CPU::INS_ADC_ABS;
    mem[0x8001] = 0x00; // Low byte of address
    mem[0x8002] = 0x70; // High byte of address
    mem[0x7000] = 0x00; // Value at address 0x7000

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.A);
    EXPECT_TRUE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
    
}

TEST_F(M6502AddCarryTest, ADCProducesZeroAndSetsCarryAndZeroFlag) {
    cpu.PC = 0x8000;
    cpu.A = 0x80;
    cpu.C = false;

    mem[0x8000] = CPU::INS_ADC_ABS;
    mem[0x8001] = 0x00;
    mem[0x8002] = 0x70;
    mem[0x7000] = 0x80;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_TRUE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_TRUE(cpu.C);   // 0x80 + 0x80 = 0x100 > 0xFF
    EXPECT_TRUE(cpu.V);   // Neg (-128) + Neg (-128) = 0 (Pos sign) -> Signed Overflow!
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502AddCarryTest, ADCProducesNegativeResultWithoutSignedOverflow) {
    cpu.PC = 0x8000;
    cpu.A = 0xFF; // -1
    cpu.C = false;

    mem[0x8000] = CPU::INS_ADC_ABS;
    mem[0x8001] = 0x00;
    mem[0x8002] = 0x70;
    mem[0x7000] = 0x01; // +1

    // -1 + 1 = 0 (Tests wrapping from negative to zero)
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_TRUE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_TRUE(cpu.C);
    EXPECT_FALSE(cpu.V);
}

TEST_F(M6502AddCarryTest, ADCPositivePlusPositiveCausesSignedOverflow) {
    cpu.PC = 0x8000;
    cpu.A = 0x40; // +64
    cpu.C = false;

    mem[0x8000] = CPU::INS_ADC_ABS;
    mem[0x8001] = 0x00;
    mem[0x8002] = 0x70;
    mem[0x7000] = 0x41; // +65

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_EQ(cpu.A, 0x81);
    EXPECT_FALSE(cpu.C);  // Sum is 0x81 <= 0xFF
    EXPECT_TRUE(cpu.V);   // Two positives made a negative
    EXPECT_TRUE(cpu.N);   // Bit 7 is set
    EXPECT_FALSE(cpu.Z);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502AddCarryTest, ADCPositivePlusPositiveMaxFitNoOverflow) {
    cpu.PC = 0x8000;
    cpu.A = 0x40; // +64
    cpu.C = false;

    mem[0x8000] = CPU::INS_ADC_ABS;
    mem[0x8001] = 0x00;
    mem[0x8002] = 0x70;
    mem[0x7000] = 0x3F; // +63

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_EQ(cpu.A, 0x7F); // +127
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.V);  // Fits in signed positive range
    EXPECT_FALSE(cpu.N);  // Bit 7 is 0
    EXPECT_FALSE(cpu.Z);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502AddCarryTest, ADCNegativePlusNegativeCausesSignedOverflow) {
    cpu.PC = 0x8000;
    cpu.A = 0x80; // -128
    cpu.C = false;

    mem[0x8000] = CPU::INS_ADC_ABS;
    mem[0x8001] = 0x00;
    mem[0x8002] = 0x70;
    mem[0x7000] = 0xFF; // -1

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_EQ(cpu.A, 0x7F); // Lower 8 bits is +127
    EXPECT_TRUE(cpu.C);   // 0x17F > 0xFF
    EXPECT_TRUE(cpu.V);   // Two negatives resulted in a positive
    EXPECT_FALSE(cpu.N);  // Bit 7 is 0
    EXPECT_FALSE(cpu.Z);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502AddCarryTest, ADCPositivePlusNegativeNeverOverflows) {
    cpu.PC = 0x8000;
    cpu.A = 0x50; // +80
    cpu.C = false;

    mem[0x8000] = CPU::INS_ADC_ABS;
    mem[0x8001] = 0x00;
    mem[0x8002] = 0x70;
    mem[0x7000] = 0xD0; // -48

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_EQ(cpu.A, 0x20); // +32
    EXPECT_TRUE(cpu.C);   // 0x50 + 0xD0 = 0x120 -> Carry set
    EXPECT_FALSE(cpu.V);  // Impossible to overflow opposing signs
    EXPECT_FALSE(cpu.N);
    EXPECT_FALSE(cpu.Z);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502AddCarryTest, ADCIncomingCarryCausesUnsignedWrap) {
    cpu.PC = 0x8000;
    cpu.A = 0xFF;
    cpu.C = true; // Incoming carry set

    mem[0x8000] = CPU::INS_ADC_ABS;
    mem[0x8001] = 0x00;
    mem[0x8002] = 0x70;
    mem[0x7000] = 0x00;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_TRUE(cpu.C);   // 0xFF + 0 + 1 = 0x100
    EXPECT_TRUE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_FALSE(cpu.V);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502AddCarryTest, ADCIncomingCarryTriggersSignedOverflow) {
    cpu.PC = 0x8000;
    cpu.A = 0x7F; // +127 (max signed positive)
    cpu.C = true; // +1

    mem[0x8000] = CPU::INS_ADC_ABS;
    mem[0x8001] = 0x00;
    mem[0x8002] = 0x70;
    mem[0x7000] = 0x00;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_EQ(cpu.A, 0x80); // Flipped to -128
    EXPECT_FALSE(cpu.C);  // Sum is 0x80 <= 0xFF
    EXPECT_TRUE(cpu.V);   // Pushed into negative territory
    EXPECT_TRUE(cpu.N);
    EXPECT_FALSE(cpu.Z);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502AddCarryTest, ADCImmediateAddsValueCorrectly) {
    cpu.PC = 0x8000;
    cpu.A = 0x20;
    cpu.C = false;

    mem[0x8000] = CPU::INS_ADC_IMM;
    mem[0x8001] = 0x15;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_EQ(cpu.A, 0x35);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_FALSE(cpu.V);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502AddCarryTest, ADCZeroPageAddsValueCorrectly) {
    cpu.PC = 0x8000;
    cpu.A = 0x10;
    cpu.C = true;

    mem[0x8000] = CPU::INS_ADC_ZP;
    mem[0x8001] = 0x42;
    mem[0x0042] = 0x25;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 3);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 3);
    EXPECT_EQ(cpu.A, 0x36);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_FALSE(cpu.V);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502AddCarryTest, ADCZeroPageXAddsValueCorrectly) {
    cpu.PC = 0x8000;
    cpu.A = 0x05;
    cpu.X = 0x08;
    cpu.C = false;

    mem[0x8000] = CPU::INS_ADC_ZP_X;
    mem[0x8001] = 0x20;
    mem[0x0028] = 0x12;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_EQ(cpu.A, 0x17);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_FALSE(cpu.V);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502AddCarryTest, ADCZeroPageXWrapsAroundZeroPage) {
    cpu.PC = 0x8000;
    cpu.A = 0x02;
    cpu.X = 0xFF;
    cpu.C = false;

    mem[0x8000] = CPU::INS_ADC_ZP_X;
    mem[0x8001] = 0x80;
    mem[0x007F] = 0x0A;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_EQ(cpu.A, 0x0C);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_FALSE(cpu.V);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502AddCarryTest, ADCAbsoluteAddsValueCorrectly) {
    cpu.PC = 0x8000;
    cpu.A = 0x30;
    cpu.C = false;

    mem[0x8000] = CPU::INS_ADC_ABS;
    mem[0x8001] = 0xCD;
    mem[0x8002] = 0xAB;
    mem[0xABCD] = 0x11;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_EQ(cpu.A, 0x41);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_FALSE(cpu.V);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502AddCarryTest, ADCAbsoluteXAddsValueSamePageTakes4Cycles) {
    cpu.PC = 0x8000;
    cpu.A = 0x10;
    cpu.X = 0x05;
    cpu.C = false;

    mem[0x8000] = CPU::INS_ADC_ABS_X;
    mem[0x8001] = 0x10;
    mem[0x8002] = 0x20;
    mem[0x2015] = 0x22;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_EQ(cpu.A, 0x32);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_FALSE(cpu.V);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502AddCarryTest, ADCAbsoluteXAddsValuePageCrossingTakes5Cycles) {
    cpu.PC = 0x8000;
    cpu.A = 0x10;
    cpu.X = 0x05;
    cpu.C = false;

    mem[0x8000] = CPU::INS_ADC_ABS_X;
    mem[0x8001] = 0xFD;
    mem[0x8002] = 0x20;
    mem[0x2102] = 0x22;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 5);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 5);
    EXPECT_EQ(cpu.A, 0x32);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_FALSE(cpu.V);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502AddCarryTest, ADCAbsoluteYAddsValueSamePageTakes4Cycles) {
    cpu.PC = 0x8000;
    cpu.A = 0x14;
    cpu.Y = 0x08;
    cpu.C = false;

    mem[0x8000] = CPU::INS_ADC_ABS_Y;
    mem[0x8001] = 0x20;
    mem[0x8002] = 0x30;
    mem[0x3028] = 0x06;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_EQ(cpu.A, 0x1A);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_FALSE(cpu.V);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502AddCarryTest, ADCAbsoluteYAddsValuePageCrossingTakes5Cycles) {
    cpu.PC = 0x8000;
    cpu.A = 0x14;
    cpu.Y = 0x08;
    cpu.C = false;

    mem[0x8000] = CPU::INS_ADC_ABS_Y;
    mem[0x8001] = 0xFA;
    mem[0x8002] = 0x30;
    mem[0x3102] = 0x06;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 5);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 5);
    EXPECT_EQ(cpu.A, 0x1A);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_FALSE(cpu.V);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502AddCarryTest, ADCIndirectXAddsValueCorrectlyTakes6Cycles) {
    cpu.PC = 0x8000;
    cpu.A = 0x03;
    cpu.X = 0x04;
    cpu.C = false;

    mem[0x8000] = CPU::INS_ADC_IND_X;
    mem[0x8001] = 0x20;
    mem[0x0024] = 0x00;
    mem[0x0025] = 0x40;
    mem[0x4000] = 0x07;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 6);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 6);
    EXPECT_EQ(cpu.A, 0x0A);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_FALSE(cpu.V);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502AddCarryTest, ADCIndirectXWrapsAroundZeroPageDuringPointerFetch) {
    cpu.PC = 0x8000;
    cpu.A = 0x05;
    cpu.X = 0x05;
    cpu.C = false;

    mem[0x8000] = CPU::INS_ADC_IND_X;
    mem[0x8001] = 0xFB;
    mem[0x0000] = 0x80;
    mem[0x0001] = 0x50;
    mem[0x5080] = 0x10;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 6);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 6);
    EXPECT_EQ(cpu.A, 0x15);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_FALSE(cpu.V);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502AddCarryTest, ADCIndirectXPointerBoundaryWrapsWithinZeroPage) {
    cpu.PC = 0x8000;
    cpu.A = 0x02;
    cpu.X = 0x00;
    cpu.C = false;

    mem[0x8000] = CPU::INS_ADC_IND_X;
    mem[0x8001] = 0xFF;
    mem[0x00FF] = 0x34;
    mem[0x0000] = 0x12;
    mem[0x1234] = 0x08;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 6);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 6);
    EXPECT_EQ(cpu.A, 0x0A);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_FALSE(cpu.V);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502AddCarryTest, ADCIndirectYSamePageTakes5Cycles) {
    cpu.PC = 0x8000;
    cpu.A = 0x01;
    cpu.Y = 0x04;
    cpu.C = false;

    mem[0x8000] = CPU::INS_ADC_IND_Y;
    mem[0x8001] = 0x40;
    mem[0x0040] = 0x10;
    mem[0x0041] = 0x60;
    mem[0x6014] = 0x09;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 5);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 5);
    EXPECT_EQ(cpu.A, 0x0A);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_FALSE(cpu.V);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502AddCarryTest, ADCIndirectYPageCrossingTakes6Cycles) {
    cpu.PC = 0x8000;
    cpu.A = 0x02;
    cpu.Y = 0x05;
    cpu.C = false;

    mem[0x8000] = CPU::INS_ADC_IND_Y;
    mem[0x8001] = 0x40;
    mem[0x0040] = 0xFD;
    mem[0x0041] = 0x60;
    mem[0x6102] = 0x08;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 6);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 6);
    EXPECT_EQ(cpu.A, 0x0A);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_FALSE(cpu.V);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}

TEST_F(M6502AddCarryTest, ADCIndirectYPointerBoundaryWrapsWithinZeroPage) {
    cpu.PC = 0x8000;
    cpu.A = 0x05;
    cpu.Y = 0x01;
    cpu.C = false;

    mem[0x8000] = CPU::INS_ADC_IND_Y;
    mem[0x8001] = 0xFF;
    mem[0x00FF] = 0x00;
    mem[0x0000] = 0x70;
    mem[0x7001] = 0x05;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 5);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 5);
    EXPECT_EQ(cpu.A, 0x0A);
    EXPECT_FALSE(cpu.C);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_FALSE(cpu.V);
    VerifyUnchangedNonArithmeticFlags(cpu, cpuCopy);
}