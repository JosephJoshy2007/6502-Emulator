#include <gtest/gtest.h>
#include "m6502.h"

class M6502InterruptTest : public ::testing::Test {
protected:
    Memory mem;
    CPU cpu;

    virtual void SetUp() override {
        cpu.Reset(mem);
    }

    virtual void TearDown() override {
    }
};

// ===========================================================================
// BRK (Break / Software Interrupt) Tests
// ===========================================================================

TEST_F(M6502InterruptTest, BRKExecutionPushesPCAndStatusSetsInterruptDisableAndTakes7Cycles) {
    cpu.PC = 0x8000;
    cpu.SP = 0xFD;
    cpu.I = false;
    cpu.D = false;
    cpu.C = true;
    cpu.Z = true;
    cpu.N = false;
    cpu.V = false;

    // Vector at 0xFFFE / 0xFFFF points to ISR at 0x9000
    mem[0xFFFE] = 0x00;
    mem[0xFFFF] = 0x90;

    mem[0x8000] = CPU::INS_BRK;
    mem[0x8001] = 0xEA; // Signature/padding byte (NOP)

    s32 cyclesUsed = cpu.Execute(mem, 7);

    EXPECT_EQ(cyclesUsed, 7);
    EXPECT_EQ(cpu.PC, 0x9000); // Redirected to interrupt vector
    EXPECT_TRUE(cpu.I);        // Interrupt disable flag must be set
    EXPECT_EQ(cpu.SP, 0xFA);   // Stack pointer moved down by 3 bytes

    // Stack Layout verification:
    // PCH pushed to 0x01FD: (0x8002 >> 8) = 0x80
    EXPECT_EQ(mem[0x01FD], 0x80);
    // PCL pushed to 0x01FC: (0x8002 & 0xFF) = 0x02
    EXPECT_EQ(mem[0x01FC], 0x02);

    // Status pushed to 0x01FB:
    // Bits: N(0) V(0) U(1) B(1) D(0) I(0) Z(1) C(1) -> 0b00110011 (0x33)
    EXPECT_EQ(mem[0x01FB] & 0x30, 0x30); // Bits 4 (B) and 5 (U) MUST be set on stack
    EXPECT_EQ(mem[0x01FB] & 0x01, 0x01); // Carry bit preserved
    EXPECT_EQ(mem[0x01FB] & 0x02, 0x02); // Zero bit preserved
}

TEST_F(M6502InterruptTest, BRKPushesExpectedPWhenOtherFlagsAreSet) {
    cpu.PC = 0x1000;
    cpu.SP = 0xFF;
    cpu.N = true;
    cpu.V = true;
    cpu.D = true;
    cpu.C = false;
    cpu.Z = false;
    cpu.I = false;

    mem[0xFFFE] = 0x50;
    mem[0xFFFF] = 0x20;

    mem[0x1000] = CPU::INS_BRK;
    mem[0x1001] = 0x00;

    s32 cyclesUsed = cpu.Execute(mem, 7);

    EXPECT_EQ(cyclesUsed, 7);
    EXPECT_EQ(cpu.PC, 0x2050);
    EXPECT_EQ(cpu.SP, 0xFC);

    // Return address: 0x1002
    EXPECT_EQ(mem[0x01FF], 0x10);
    EXPECT_EQ(mem[0x01FE], 0x02);

    // Flags pushed: N(1) V(1) U(1) B(1) D(1) I(0) Z(0) C(0) -> 0b11111000 (0xF8)
    EXPECT_EQ(mem[0x01FD], 0xF8);
    EXPECT_TRUE(cpu.I); // I is set inside CPU after vectoring
}

// ===========================================================================
// RTI (Return from Interrupt) Tests
// ===========================================================================

TEST_F(M6502InterruptTest, RTIRestoresFlagsAndPCFromStackTakes6Cycles) {
    cpu.PC = 0x9000;
    cpu.SP = 0xFA; // Stack with 3 bytes pushed
    cpu.I = true;
    cpu.C = false;
    cpu.Z = false;

    // Place simulated stack frames
    // Status at 0x01FB: N(0) V(1) U(1) B(1) D(0) I(0) Z(1) C(1) -> 0x73
    mem[0x01FB] = 0x73;
    // PCL at 0x01FC: 0x05
    mem[0x01FC] = 0x05;
    // PCH at 0x01FD: 0x80
    mem[0x01FD] = 0x80;

    mem[0x9000] = CPU::INS_RTI;

    s32 cyclesUsed = cpu.Execute(mem, 6);

    EXPECT_EQ(cyclesUsed, 6);
    EXPECT_EQ(cpu.PC, 0x8005); // Restored to 0x8005
    EXPECT_EQ(cpu.SP, 0xFD);   // SP incremented by 3
    EXPECT_TRUE(cpu.C);
    EXPECT_TRUE(cpu.Z);
    EXPECT_FALSE(cpu.I);       // Restored I=0 from stack status
    EXPECT_TRUE(cpu.V);
    EXPECT_FALSE(cpu.N);
}

TEST_F(M6502InterruptTest, RTIStackWrapAroundZeroPageStack) {
    cpu.PC = 0x9000;
    // Test 8-bit unsigned SP wrap around boundary 0xFE -> 0xFF -> 0x00 -> 0x01
    cpu.SP = 0xFE;

    mem[0x01FF] = 0x00; // Status
    mem[0x0100] = 0x34; // PCL
    mem[0x0101] = 0x12; // PCH

    mem[0x9000] = CPU::INS_RTI;

    s32 cyclesUsed = cpu.Execute(mem, 6);

    EXPECT_EQ(cyclesUsed, 6);
    EXPECT_EQ(cpu.PC, 0x1234);
    EXPECT_EQ(cpu.SP, 0x01);
}

// ===========================================================================
// Full Round-Trip: BRK into ISR followed by RTI
// ===========================================================================

TEST_F(M6502InterruptTest, BRKFollowedByRTICompletesFullRoundTrip) {
    cpu.PC = 0x8000;
    cpu.SP = 0xFD;
    cpu.A = 0x42;
    cpu.X = 0x10;
    cpu.Y = 0x20;
    cpu.C = true;
    cpu.Z = false;
    cpu.I = false;
    cpu.D = false;
    cpu.V = false;
    cpu.N = false;

    // Vector points to ISR at 0xC000
    mem[0xFFFE] = 0x00;
    mem[0xFFFF] = 0xC0;

    // Main program
    mem[0x8000] = CPU::INS_BRK;
    mem[0x8001] = 0x00; // Padding byte
    mem[0x8002] = CPU::INS_NOP;

    // ISR code: RTI
    mem[0xC000] = CPU::INS_RTI;

    // Step 1: Execute BRK (7 cycles)
    s32 brkCycles = cpu.Execute(mem, 7);
    EXPECT_EQ(brkCycles, 7);
    EXPECT_EQ(cpu.PC, 0xC000);
    EXPECT_TRUE(cpu.I);
    EXPECT_EQ(cpu.SP, 0xFA);

    // Step 2: Execute RTI in ISR (6 cycles)
    s32 rtiCycles = cpu.Execute(mem, 6);
    EXPECT_EQ(rtiCycles, 6);
    EXPECT_EQ(cpu.PC, 0x8002); // Successfully returned to next instruction past BRK signature
    EXPECT_EQ(cpu.SP, 0xFD);   // SP returned to initial baseline
    EXPECT_FALSE(cpu.I);       // Restored I=false from original status
    EXPECT_TRUE(cpu.C);        // Restored C=true
    EXPECT_FALSE(cpu.Z);
    EXPECT_EQ(cpu.A, 0x42);    // Registers preserved
    EXPECT_EQ(cpu.X, 0x10);
    EXPECT_EQ(cpu.Y, 0x20);
}