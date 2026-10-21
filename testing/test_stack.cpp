#include <gtest/gtest.h>
#include "m6502.h"

// The test fixture sets up a fresh CPU and Memory before every single test
class M6502StackTest : public ::testing::Test {
protected:
    Memory mem;
    CPU cpu;

    virtual void SetUp() override {
        // Reset the CPU exactly as done at the start of a program
        cpu.Reset(mem); 
    }

    virtual void TearDown() override {
        // Optional cleanup
    }
};

void VerifyAllUnchangedFlags(const CPU& current, const CPU& original) {
    SCOPED_TRACE("Verifying unchanged status flags (C, B, D, I, V, Z, N)");
    EXPECT_EQ(current.C, original.C);
    EXPECT_EQ(current.B, original.B);
    EXPECT_EQ(current.D, original.D);
    EXPECT_EQ(current.I, original.I);
    EXPECT_EQ(current.V, original.V);
    EXPECT_EQ(current.Z, original.Z);
    EXPECT_EQ(current.N, original.N);
}

void VerifyUnchangedFlags(const CPU& current, const CPU& original) {
    SCOPED_TRACE("Verifying unchanged status flags (C, B, D, I, V)");
    EXPECT_EQ(current.C, original.C);
    EXPECT_EQ(current.B, original.B);
    EXPECT_EQ(current.D, original.D);
    EXPECT_EQ(current.I, original.I);
    EXPECT_EQ(current.V, original.V);
}

TEST_F(M6502StackTest, TSXCanTransferStackPointerToX) {
    cpu.SP = 0xAB; // Set SP to a known value
    mem[0xFFFC] = CPU::INS_TSX; // TSX instruction

    CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cyclesUsed, 2);
    
    EXPECT_EQ(cpu.X, cpu.SP); // X should now equal the original SP value
    EXPECT_FALSE(cpu.Z); // Z flag should remain unchanged
    EXPECT_TRUE(cpu.N); // N flag should remain unchanged
    VerifyUnchangedFlags(cpu, cpuCopy);
}

TEST_F(M6502StackTest, TXSCanTransferXToStackPointer) {
    cpu.X = 0xCD; // Set X to a known value
    mem[0xFFFC] = CPU::INS_TXS; // TXS instruction

    CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cyclesUsed, 2);
    
    EXPECT_EQ(cpu.SP, cpu.X); // SP should now equal the original X value
    VerifyAllUnchangedFlags(cpu, cpuCopy);
}

TEST_F(M6502StackTest, PHACanPushAccumulatorOntoStack) {
    cpu.A = 0x42;
    mem[0xFFFC] = CPU::INS_PHA;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 3);

    EXPECT_EQ(cyclesUsed, 3);
    EXPECT_EQ(cpu.SP, static_cast<Byte>(cpuCopy.SP - 1));

    // Stack is located in Page 1 (0x0100 - 0x01FF)
    Word stackAddress = 0x0100 | static_cast<Byte>(cpu.SP + 1);
    EXPECT_EQ(mem[stackAddress], 0x42);

    VerifyAllUnchangedFlags(cpu, cpuCopy);
}

TEST_F(M6502StackTest, PLACanPullAccumilatorFromStack) {
    cpu.A = 0x00; // Clear A
    cpu.SP = 0xFE; // Set SP to a known value
    mem[0xFFFC] = CPU::INS_PLA; // PLA instruction
    mem[0x01FF] = 0x99; // Push a known value onto the stack

    CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cyclesUsed, 4);
    
    EXPECT_EQ(cpu.A, 0x99); // A should now equal the value pulled from the stack
    EXPECT_EQ(cpu.SP, cpuCopy.SP + 1); // SP should increment by 1
    VerifyUnchangedFlags(cpu, cpuCopy);
}

TEST_F(M6502StackTest, PLACanSetZeroAndNegativeFlags) {
    cpu.A = 0x01; // Clear A
    cpu.SP = 0xFE; // Set SP to a known value
    mem[0xFFFC] = CPU::INS_PLA; // PLA instruction
    mem[0x01FF] = 0x00; // Push zero onto the stack

    CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cyclesUsed, 4);
    
    EXPECT_EQ(cpu.A, 0x00); // A should now equal the value pulled from the stack
    EXPECT_TRUE(cpu.Z); // Z flag should be set
    EXPECT_FALSE(cpu.N); // N flag should be clear

    // Now test pulling a negative value
    cpu.SP = 0xFE; // Reset SP
    mem[0x01FF] = 0x80; // Push a negative value onto the stack
    mem[0xFFFD] = CPU::INS_PLA;

    cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cyclesUsed, 4);
    
    EXPECT_EQ(cpu.A, 0x80); // A should now equal the value pulled from the stack
    EXPECT_FALSE(cpu.Z); // Z flag should be clear
    EXPECT_TRUE(cpu.N); // N flag should be set

    VerifyUnchangedFlags(cpu, cpuCopy);
}

TEST_F(M6502StackTest, StackPointerWrapsAround) {
    cpu.SP = 0x00; // Set SP to the bottom of the stack
    mem[0xFFFC] = CPU::INS_PHA; // PHA instruction

    CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 3);

    EXPECT_EQ(cyclesUsed, 3);
    
    EXPECT_EQ(cpu.SP, 0xFF); // SP should wrap around to the top of the stack
    VerifyAllUnchangedFlags(cpu, cpuCopy);
}

TEST_F(M6502StackTest, StackPointerWrapsAroundOnPull) {
    cpu.SP = 0xFF; // Set SP to the top of the stack
    mem[0xFFFC] = CPU::INS_PLA; // PLA instruction
    mem[0x0100] = 0x55; // Push a known value onto the stack

    CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cyclesUsed, 4);
    
    EXPECT_EQ(cpu.SP, 0x00); // SP should wrap around to the bottom of the stack
    EXPECT_EQ(cpu.A, 0x55); // A should now equal the value pulled from the stack
    VerifyAllUnchangedFlags(cpu, cpuCopy);
}

TEST_F(M6502StackTest, PHPAndPLPCanPushAndPullProcessorStatus) {
    cpu.SetStatusRegister(0b11010101); // Set a known status
    mem[0xFFFC] = CPU::INS_PHP; // PHP instruction

    CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 3);

    EXPECT_EQ(cyclesUsed, 3);
    
    Word stackAddress = 0x0100 | static_cast<Byte>(cpu.SP + 1);
    EXPECT_EQ(mem[stackAddress], cpuCopy.GetStatusRegister()); // The status should be pushed onto the stack
    EXPECT_EQ(cpu.SP, static_cast<Byte>(cpuCopy.SP - 1)); // SP should decrement by 1

    // Now test pulling the status back
    mem[0xFFFD] = CPU::INS_PLP; // PLP instruction
    cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cyclesUsed, 4);
    
    EXPECT_EQ(cpu.GetStatusRegister(), cpuCopy.GetStatusRegister()); // Status should be restored
    EXPECT_EQ(cpu.SP, cpuCopy.SP); // SP should return to original value
}

