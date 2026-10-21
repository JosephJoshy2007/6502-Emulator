#include <gtest/gtest.h>
#include "m6502.h"

// The test fixture sets up a fresh CPU and Memory before every single test
class M6502LoadTest : public ::testing::Test {
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

void VerifyUnchangedFlagsForLoad(const CPU& current, const CPU& original) {
    SCOPED_TRACE("Verifying unchanged status flags (C, B, D, I, V)");
    EXPECT_EQ(current.C, original.C);
    EXPECT_EQ(current.B, original.B);
    EXPECT_EQ(current.D, original.D);
    EXPECT_EQ(current.I, original.I);
    EXPECT_EQ(current.V, original.V);
}

void TestLoadRegisterImmediate(CPU& cpu, Memory& mem, Byte opcode,Byte CPU::*Register) {

    cpu.Z = cpu.N = true;

    mem[0xFFFC] = opcode; 
    mem[0xFFFD] = 0x84; 
    
    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);
    
    EXPECT_EQ(cyclesUsed, 2);

    // Then: The A register should hold the value and flags should update
    EXPECT_EQ(cpu.*Register, 0x84);
    EXPECT_FALSE(cpu.Z); // 0x84 is not zero
    EXPECT_TRUE(cpu.N);  // bit 7 is set in 0x84, meaning negative
    VerifyUnchangedFlagsForLoad(cpu, cpuCopy);
}

void TestLoadRegisterZeroPage(CPU& cpu, Memory& mem, Byte opcode, Byte CPU::*Register) {

    cpu.Z = cpu.N = true;

    mem[0xFFFC] = opcode; 
    mem[0xFFFD] = 0x10; 
    mem[0x0010] = 0x42; // The value at zero page address 0x10 is 0x42
    
    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 3);
    
    EXPECT_EQ(cyclesUsed, 3);
    
    EXPECT_EQ(cpu.*Register, 0x42);
    EXPECT_FALSE(cpu.Z); // 0x42 is not zero
    EXPECT_FALSE(cpu.N); // bit 7 is not set in 0x42, meaning not negative
    VerifyUnchangedFlagsForLoad(cpu, cpuCopy);
}

void TestLoadRegisterZeroPageWithXOffset(CPU& cpu, Memory& mem, Byte opcode, Byte CPU::*Register) {

    cpu.Z = cpu.N = true;

    mem[0xFFFC] = opcode; 
    mem[0xFFFD] = 0x80;  // Address 0x0080
    mem[0x0085] = 0x42;
    cpu.X = 0x05;
    
    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);
    
    EXPECT_EQ(cyclesUsed, 4);
    
    EXPECT_EQ(cpu.*Register, 0x42);
    EXPECT_FALSE(cpu.Z); // 0x42 is not zero
    EXPECT_FALSE(cpu.N); // bit 7 is not set in 0x42, meaning not negative
    VerifyUnchangedFlagsForLoad(cpu, cpuCopy);
}

void TestLoadRegisterZeroPageWithYOffset(CPU& cpu, Memory& mem, Byte opcode, Byte CPU::*Register) {

    cpu.Z = cpu.N = true;

    mem[0xFFFC] = opcode; 
    mem[0xFFFD] = 0x80;  // Address 0x0080
    mem[0x0085] = 0x42;
    cpu.Y = 0x05;
    
    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);
    
    EXPECT_EQ(cyclesUsed, 4);
    
    EXPECT_EQ(cpu.*Register, 0x42);
    EXPECT_FALSE(cpu.Z); // 0x42 is not zero
    EXPECT_FALSE(cpu.N); // bit 7 is not set in 0x42, meaning not negative
    VerifyUnchangedFlagsForLoad(cpu, cpuCopy);
}

void TestLoadRegisterAbsolute(CPU& cpu, Memory& mem, Byte opcode, Byte CPU::*Register) {

    cpu.Z = cpu.N = true;

    mem[0xFFFC] = opcode; 
    mem[0xFFFD] = 0x00; 
    mem[0xFFFE] = 0x80; // Address 0x8000
    mem[0x8000] = 0x42;
    
    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);
    
    EXPECT_EQ(cyclesUsed, 4);
    
    EXPECT_EQ(cpu.*Register, 0x42);
    EXPECT_FALSE(cpu.Z); // 0x42 is not zero
    EXPECT_FALSE(cpu.N); // bit 7 is not set in 0x42, meaning not negative
    VerifyUnchangedFlagsForLoad(cpu, cpuCopy);
}

void TestLoadRegisterAbsoluteWithXOffset(CPU& cpu, Memory& mem, Byte opcode, Byte CPU::*Register) {

    cpu.Z = cpu.N = true;

    mem[0xFFFC] = opcode; 
    mem[0xFFFD] = 0x00; 
    mem[0xFFFE] = 0x80; // Address 0x8000
    mem[0x8005] = 0x42;
    cpu.X = 0x05;
    
    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);
    
    EXPECT_EQ(cyclesUsed, 4);
    
    EXPECT_EQ(cpu.*Register, 0x42);
    EXPECT_FALSE(cpu.Z); // 0x42 is not zero
    EXPECT_FALSE(cpu.N); // bit 7 is not set in 0x42, meaning not negative
    VerifyUnchangedFlagsForLoad(cpu, cpuCopy);
}

void TestLoadRegisterAbsoluteWithYOffset(CPU& cpu, Memory& mem, Byte opcode, Byte CPU::*Register) {

    cpu.Z = cpu.N = true;

    mem[0xFFFC] = opcode; 
    mem[0xFFFD] = 0x00; 
    mem[0xFFFE] = 0x80; // Address 0x8000
    mem[0x8005] = 0x42;
    cpu.Y = 0x05;
    
    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);
    
    EXPECT_EQ(cyclesUsed, 4);
    
    EXPECT_EQ(cpu.*Register, 0x42);
    EXPECT_FALSE(cpu.Z); // 0x42 is not zero
    EXPECT_FALSE(cpu.N); // bit 7 is not set in 0x42, meaning not negative
    VerifyUnchangedFlagsForLoad(cpu, cpuCopy);
}

void TestLoadRegisterAbsoluteWithXOffsetAndPageCrossing(CPU& cpu, Memory& mem, Byte opcode, Byte CPU::*Register) {
    cpu.Z = cpu.N = true;
    mem[0xFFFC] = opcode; 
    mem[0xFFFD] = 0xF0; 
    mem[0xFFFE] = 0x80; // Address 0x80F0
    mem[0x8105] = 0x42; // Value at address 0x8105 (after adding X offset)
    cpu.X = 0x15; // X offset that causes page crossing
    
    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 5); // Should take an extra cycle due to page crossing
    
    EXPECT_EQ(cyclesUsed, 5);
    
    EXPECT_EQ(cpu.*Register, 0x42);
    EXPECT_FALSE(cpu.Z); // 0x42 is not zero
    EXPECT_FALSE(cpu.N); // bit 7 is not set in 0x42, meaning not negative
    VerifyUnchangedFlagsForLoad(cpu, cpuCopy);
}

void TestLoadRegisterAbsoluteWithYOffsetAndPageCrossing(CPU& cpu, Memory& mem, Byte opcode, Byte CPU::*Register) {
    cpu.Z = cpu.N = true;
    mem[0xFFFC] = opcode; 
    mem[0xFFFD] = 0xF0; 
    mem[0xFFFE] = 0x80; // Address 0x80F0
    mem[0x8105] = 0x42; // Value at address 0x8105 (after adding Y offset)
    cpu.Y = 0x15; // Y offset that causes page crossing
    
    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 5); // Should take an extra cycle due to page crossing
    
    EXPECT_EQ(cyclesUsed, 5);
    
    EXPECT_EQ(cpu.*Register, 0x42);
    EXPECT_FALSE(cpu.Z); // 0x42 is not zero
    EXPECT_FALSE(cpu.N); // bit 7 is not set in 0x42, meaning not negative
    VerifyUnchangedFlagsForLoad(cpu, cpuCopy);
}


TEST_F(M6502LoadTest, CPUDoesNothingWhenNoInstructionsAreExecuted) {
    CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 0);
    
    EXPECT_EQ(cyclesUsed, 0);
    EXPECT_EQ(cpu.PC, cpuCopy.PC);
    EXPECT_EQ(cpu.SP, cpuCopy.SP);
    EXPECT_EQ(cpu.A, cpuCopy.A);
    EXPECT_EQ(cpu.X, cpuCopy.X);
    EXPECT_EQ(cpu.Y, cpuCopy.Y);
    VerifyUnchangedFlagsForLoad(cpu, cpuCopy);
}

TEST_F(M6502LoadTest, ExecutingABadInstructionDoesNotCauseInfiniteLoop) {
    mem[0xFFFC] = 0x00; // Invalid opcode
    mem[0xFFFD] = 0x00; // Just to have some data after the invalid opcode
    CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 1);
    
    EXPECT_EQ(cyclesUsed, 1);
    EXPECT_EQ(cpu.PC, 0xFFFD); // PC should have advanced past the invalid opcode
}


// A simple test for the Load Accumulator (LDA) Immediate instruction
TEST_F(M6502LoadTest, LDAImmediateCanLoadValueIntoARegister) {
    // Given: Program memory set up with an LDA instruction (0xA9) and a value (0x84)
    TestLoadRegisterImmediate(cpu, mem, CPU::INS_LDA_IMM, &CPU::A);
}

TEST_F(M6502LoadTest, LDXImmediateCanLoadValueIntoXRegister) {
    // Given: Program memory set up with an LDX instruction (0xA2) and a value (0x84)
    TestLoadRegisterImmediate(cpu, mem, CPU::INS_LDX_IMM, &CPU::X);
}

TEST_F(M6502LoadTest, LDYImmediateCanLoadValueIntoYRegister) {
    // Given: Program memory set up with an LDY instruction (0xA0) and a value (0x84)
    TestLoadRegisterImmediate(cpu, mem, CPU::INS_LDY_IMM, &CPU::Y);
}

TEST_F(M6502LoadTest, LDAZeroPageCanLoadValueIntoARegister) {
    
    TestLoadRegisterZeroPage(cpu, mem, CPU::INS_LDA_ZP, &CPU::A);
}

TEST_F(M6502LoadTest, LDXZeroPageCanLoadValueIntoXRegister) {
    
    TestLoadRegisterZeroPage(cpu, mem, CPU::INS_LDX_ZP, &CPU::X);
}

TEST_F(M6502LoadTest, LDYZeroPageCanLoadValueIntoYRegister) {
    
    TestLoadRegisterZeroPage(cpu, mem, CPU::INS_LDY_ZP, &CPU::Y);
}

TEST_F(M6502LoadTest, LDAZeroPageXCanLoadValueIntoARegister) {
    
    TestLoadRegisterZeroPageWithXOffset(cpu, mem, CPU::INS_LDA_ZP_X, &CPU::A);
}

TEST_F(M6502LoadTest, LDXZeroPageYCanLoadValueIntoXRegister) {
    
    TestLoadRegisterZeroPageWithYOffset(cpu, mem, CPU::INS_LDX_ZP_Y, &CPU::X);
}

TEST_F(M6502LoadTest, LDYZeroPageXCanLoadValueIntoYRegister) {
    
    TestLoadRegisterZeroPageWithXOffset(cpu, mem, CPU::INS_LDY_ZP_X, &CPU::Y);
}

TEST_F(M6502LoadTest, LDAAbsoluteCanLoadValueIntoARegister) {
    
    TestLoadRegisterAbsolute(cpu, mem, CPU::INS_LDA_ABS, &CPU::A);
}

TEST_F(M6502LoadTest, LDXAbsoluteCanLoadValueIntoXRegister) {
    
    TestLoadRegisterAbsolute(cpu, mem, CPU::INS_LDX_ABS, &CPU::X);
}

TEST_F(M6502LoadTest, LDYAbsoluteCanLoadValueIntoYRegister) {
    
    TestLoadRegisterAbsolute(cpu, mem, CPU::INS_LDY_ABS, &CPU::Y);
}


TEST_F(M6502LoadTest, LDAAbsoluteXCanLoadIntoARegister){

    TestLoadRegisterAbsoluteWithXOffset(cpu, mem, CPU::INS_LDA_ABS_X, &CPU::A);
}

TEST_F(M6502LoadTest, LDAAbsoluteYCanLoadIntoARegister){

    TestLoadRegisterAbsoluteWithYOffset(cpu, mem, CPU::INS_LDA_ABS_Y, &CPU::A);
}

TEST_F(M6502LoadTest, LDXAbsoluteYCanLoadIntoXRegister){

    TestLoadRegisterAbsoluteWithYOffset(cpu, mem, CPU::INS_LDX_ABS_Y, &CPU::X);
}

TEST_F(M6502LoadTest, LDYAbsoluteXCanLoadIntoYRegister){

    TestLoadRegisterAbsoluteWithXOffset(cpu, mem, CPU::INS_LDY_ABS_X, &CPU::Y);
}

TEST_F(M6502LoadTest, LDAAbsoluteXCanLoadIntoARegisterWithPageCrossing){

    TestLoadRegisterAbsoluteWithXOffsetAndPageCrossing(cpu, mem, CPU::INS_LDA_ABS_X, &CPU::A);
}

TEST_F(M6502LoadTest, LDAAbsoluteYCanLoadIntoARegisterWithPageCrossing){

    TestLoadRegisterAbsoluteWithYOffsetAndPageCrossing(cpu, mem, CPU::INS_LDA_ABS_Y, &CPU::A);
}

TEST_F(M6502LoadTest, LDXAbsoluteYCanLoadIntoXRegisterWithPageCrossing){

    TestLoadRegisterAbsoluteWithYOffsetAndPageCrossing(cpu, mem, CPU::INS_LDX_ABS_Y, &CPU::X);
}

TEST_F(M6502LoadTest, LDYAbsoluteXCanLoadIntoYRegisterWithPageCrossing){

    TestLoadRegisterAbsoluteWithXOffsetAndPageCrossing(cpu, mem, CPU::INS_LDY_ABS_X, &CPU::Y);
}

TEST_F(M6502LoadTest, LDAIndirectXCanLoadIntoARegister) {
    
    mem[0xFFFC] = CPU::INS_LDA_IND_X; 
    mem[0xFFFD] = 0x10; 
    cpu.X = 0x04; // X register is 4
    mem[0x0014] = 0x00; // Low byte of effective address
    mem[0x0015] = 0x80; // High byte of effective address
    mem[0x8000] = 0x55; // Value at effective address
    
    CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 6);

    EXPECT_EQ(cyclesUsed, 6);
    
    EXPECT_EQ(cpu.A, 0x55);
    EXPECT_FALSE(cpu.Z); // 0x55 is not zero
    EXPECT_FALSE(cpu.N); // bit 7 is not set in 0x55, meaning not negative
    VerifyUnchangedFlagsForLoad(cpu, cpuCopy);
}

TEST_F(M6502LoadTest, LDAIndirectYCanLoadIntoARegister) {
    
    mem[0xFFFC] = CPU::INS_LDA_IND_Y; 
    mem[0xFFFD] = 0x10; 
    cpu.Y = 0x04; // Y register is 4
    mem[0x0010] = 0x00; // Low byte of base address
    mem[0x0011] = 0x80; // High byte of base address
    mem[0x8004] = 0x66; // Value at effective address (base + Y)
    
    CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 5);

    EXPECT_EQ(cyclesUsed, 5);
    
    EXPECT_EQ(cpu.A, 0x66);
    EXPECT_FALSE(cpu.Z); // 0x66 is not zero
    EXPECT_FALSE(cpu.N); // bit 7 is not set in 0x66, meaning not negative
    VerifyUnchangedFlagsForLoad(cpu, cpuCopy);
}

TEST_F(M6502LoadTest, LDAIndirectYCanLoadIntoARegisterWithPageCrossing) {
    
    mem[0xFFFC] = CPU::INS_LDA_IND_Y; 
    mem[0xFFFD] = 0x10; 
    cpu.Y = 0x15; // Y register is 21
    mem[0x0010] = 0xF0; // Low byte of base address
    mem[0x0011] = 0x80; // High byte of base address
    mem[0x8105] = 0x77; // Value at effective address (base + Y)
    
    CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 6); // Should take an extra cycle due to page crossing

    EXPECT_EQ(cyclesUsed, 6);
    
    EXPECT_EQ(cpu.A, 0x77);
    EXPECT_FALSE(cpu.Z); // 0x77 is not zero
    EXPECT_FALSE(cpu.N); // bit 7 is not set in 0x77, meaning not negative
    VerifyUnchangedFlagsForLoad(cpu, cpuCopy);
}
