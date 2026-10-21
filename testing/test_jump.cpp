#include <gtest/gtest.h>
#include "m6502.h"

// The test fixture sets up a fresh CPU and Memory before every single test
class M6502JumpTest : public ::testing::Test {
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

void VerifyUnchangedFlags(const CPU& current, const CPU& original) {
    SCOPED_TRACE("Verifying unchanged status flags (C, B, D, I, V, Z, N)");
    EXPECT_EQ(current.C, original.C);
    EXPECT_EQ(current.B, original.B);
    EXPECT_EQ(current.D, original.D);
    EXPECT_EQ(current.I, original.I);
    EXPECT_EQ(current.V, original.V);
    EXPECT_EQ(current.Z, original.Z);
    EXPECT_EQ(current.N, original.N);
}

TEST_F(M6502JumpTest, JMPABSCanJump) {
    
    mem[0xFFFC] = CPU::INS_JMP_ABS; 
    mem[0xFFFD] = 0x00; 
    mem[0xFFFE] = 0x80; // Jump to address 0x8000
    
    CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 3);

    EXPECT_EQ(cyclesUsed, 3);
    
    EXPECT_EQ(cpu.PC, 0x8000); // After JMP, PC should point to the jump address
    EXPECT_FALSE(cpu.Z); // Z flag should remain unchanged
    EXPECT_FALSE(cpu.N); // N flag should remain unchanged
    VerifyUnchangedFlags(cpu, cpuCopy);
}

TEST_F(M6502JumpTest, JSRCanJumpAndPushReturnAddress) {
    
    mem[0xFFFC] = CPU::INS_JSR; 
    mem[0xFFFD] = 0x00; 
    mem[0xFFFE] = 0x80; // Jump to address 0x8000
    
    CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 6);

    EXPECT_EQ(cyclesUsed, 6);
    
    EXPECT_EQ(cpu.PC, 0x8000); // After JSR, PC should point to the jump address
    EXPECT_EQ(cpu.SP, 0xFD); // Stack pointer should have decremented by 2 (return address pushed)
    
    // The return address (address of the next instruction after JSR) should be on the stack
    Word returnAddress = (mem[0x01FF] << 8) | mem[0x01FE]; // Stack is at 0x0100 + SP
    EXPECT_EQ(returnAddress, 0xFFFE); // Return address should be the address of the instruction after JSR
    
    VerifyUnchangedFlags(cpu, cpuCopy);
}

TEST_F(M6502JumpTest, RTSCanReturnFromSubroutine) {
    
    // Set up a JSR to 0x8000
    mem[0xFFFC] = CPU::INS_JSR; 
    mem[0xFFFD] = 0x00; 
    mem[0xFFFE] = 0x80; // Jump to address 0x8000
    
    // At 0x8000, place an RTS instruction
    mem[0x8000] = CPU::INS_RTS; 
    
    CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 12); // Enough cycles to execute JSR and RTS

    EXPECT_EQ(cyclesUsed, 12);
    
    EXPECT_EQ(cpu.PC, 0xFFFF); // After RTS, PC should point to the instruction after JSR (which is at 0xFFFF)
    EXPECT_EQ(cpu.SP, 0xFF); // Stack pointer should be back to its original value after popping return address
    
    VerifyUnchangedFlags(cpu, cpuCopy);
}

TEST_F(M6502JumpTest, JMPINDCanJump) {
    
    // Set up a JMP (indirect) to address stored at 0x8000
    mem[0xFFFC] = CPU::INS_JMP_IND; 
    mem[0xFFFD] = 0x00; 
    mem[0xFFFE] = 0x80; // Pointer to the address to jump to
    
    // Store the target address (0x9000) at 0x8000
    mem[0x8000] = 0x00; // Low byte of target address
    mem[0x8001] = 0x90; // High byte of target address
    
    CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 5);

    EXPECT_EQ(cyclesUsed, 5);
    
    EXPECT_EQ(cpu.PC, 0x9000); // After JMP (indirect), PC should point to the target address
    VerifyUnchangedFlags(cpu, cpuCopy);
}
