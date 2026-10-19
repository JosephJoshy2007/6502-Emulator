#include <gtest/gtest.h>
#include "m6502.h"

// The test fixture sets up a fresh CPU and Memory before every single test
class M6502Test : public ::testing::Test {
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
    SCOPED_TRACE("Verifying unchanged status flags (C, B, D, I, V, Z, N)");
    EXPECT_EQ(current.C, original.C);
    EXPECT_EQ(current.B, original.B);
    EXPECT_EQ(current.D, original.D);
    EXPECT_EQ(current.I, original.I);
    EXPECT_EQ(current.V, original.V);
    EXPECT_EQ(current.Z, original.Z);
    EXPECT_EQ(current.N, original.N);
}

void TestStoreRegisterZeroPage(CPU& cpu, Memory& mem, Byte opcode, Byte CPU::*Register) {

    mem[0xFFFC] = opcode; 
    mem[0xFFFD] = 0x10; // Address 0x10
    cpu.*Register = 0x42; // The register holds the value to store
    
    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 3);
    
    EXPECT_EQ(cyclesUsed, 3);
    
    EXPECT_EQ(mem[0x0010], 0x42); // Memory at address 0x10 should now hold the value from the register
    VerifyUnchangedFlagsForLoad(cpu, cpuCopy);
}

void TestStoreRegisterWithXOffset(CPU& cpu, Memory& mem, Byte opcode, Byte CPU::*Register) {

    mem[0xFFFC] = opcode; 
    mem[0xFFFD] = 0x10; // Base address 0x10
    cpu.X = 0x05; // X offset
    cpu.*Register = 0x55; // The register holds the value to store
    
    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);
    
    EXPECT_EQ(cyclesUsed, 4);
    
    EXPECT_EQ(mem[0x0015], 0x55); // Memory at address 0x15 (0x10 + X) should now hold the value from the register
    VerifyUnchangedFlagsForLoad(cpu, cpuCopy);
}

void TestStoreRegisterWithYOffset(CPU& cpu, Memory& mem, Byte opcode, Byte CPU::*Register) {

    mem[0xFFFC] = opcode; 
    mem[0xFFFD] = 0x10; // Base address 0x10
    cpu.Y = 0x05; // X offset
    cpu.*Register = 0x55; // The register holds the value to store
    
    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);
    
    EXPECT_EQ(cyclesUsed, 4);
    
    EXPECT_EQ(mem[0x0015], 0x55); // Memory at address 0x15 (0x10 + Y) should now hold the value from the register
    VerifyUnchangedFlagsForLoad(cpu, cpuCopy);
}

void TestStoreRegisterAbsolute(CPU& cpu, Memory& mem, Byte opcode, Byte CPU::*Register) {

    mem[0xFFFC] = opcode; 
    mem[0xFFFD] = 0x00; // Low byte of address
    mem[0xFFFE] = 0x20; // High byte of address (address 0x2000)
    cpu.*Register = 0x99; // The register holds the value to store
    
    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);
    
    EXPECT_EQ(cyclesUsed, 4);
    
    EXPECT_EQ(mem[0x2000], 0x99); // Memory at address 0x2000 should now hold the value from the register
    VerifyUnchangedFlagsForLoad(cpu, cpuCopy);
}

TEST_F(M6502Test, STAZeroPageCanStoreARegisterIntoMemory) {
    TestStoreRegisterZeroPage(cpu, mem, CPU::INS_STA_ZP, &CPU::A);
}

TEST_F(M6502Test, STXZeroPageCanStoreARegisterIntoMemory) {
    TestStoreRegisterZeroPage(cpu, mem, CPU::INS_STX_ZP, &CPU::X);
}

TEST_F(M6502Test, STYZeroPageCanStoreARegisterIntoMemory) {
    TestStoreRegisterZeroPage(cpu, mem, CPU::INS_STY_ZP, &CPU::Y);
}

TEST_F(M6502Test, STAZeroPageXCanStoreARegisterIntoMemory) {
    TestStoreRegisterWithXOffset(cpu, mem, CPU::INS_STA_ZP_X, &CPU::A);
}

TEST_F(M6502Test, STXZeroPageYCanStoreARegisterIntoMemory) {
    TestStoreRegisterWithYOffset(cpu, mem, CPU::INS_STX_ZP_Y, &CPU::X);
}

TEST_F(M6502Test, STYZeroPageXCanStoreARegisterIntoMemory) {
    TestStoreRegisterWithXOffset(cpu, mem, CPU::INS_STY_ZP_X, &CPU::Y);
}

TEST_F(M6502Test, STAAbsoluteCanStoreARegisterIntoMemory) {
    TestStoreRegisterAbsolute(cpu, mem, CPU::INS_STA_ABS, &CPU::A);
}

TEST_F(M6502Test, STXAbsoluteCanStoreARegisterIntoMemory) {
    TestStoreRegisterAbsolute(cpu, mem, CPU::INS_STX_ABS, &CPU::X);
}

TEST_F(M6502Test, STYAbsoluteCanStoreARegisterIntoMemory) {
    TestStoreRegisterAbsolute(cpu, mem, CPU::INS_STY_ABS, &CPU::Y);
}

TEST_F(M6502Test, STAAbsoluteWithXOffsetCanStoreARegisterIntoMemory) {
    mem[0xFFFC] = CPU::INS_STA_ABS_X;
    mem[0xFFFD] = 0x00;
    mem[0xFFFE] = 0x80;
    cpu.X = 0x05;
    cpu.A = 0x99;
    
    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 5);
    
    EXPECT_EQ(cyclesUsed, 5);
    
    EXPECT_EQ(mem[0x8005], 0x99);
    VerifyUnchangedFlagsForLoad(cpu, cpuCopy);
}

TEST_F(M6502Test, STAAbsoluteWithYOffsetCanStoreARegisterIntoMemory) {
    mem[0xFFFC] = CPU::INS_STA_ABS_Y;
    mem[0xFFFD] = 0x00;
    mem[0xFFFE] = 0x80;
    cpu.Y = 0x05;
    cpu.A = 0x99;
    
    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 5);
    
    EXPECT_EQ(cyclesUsed, 5);
    
    EXPECT_EQ(mem[0x8005], 0x99);
    VerifyUnchangedFlagsForLoad(cpu, cpuCopy);
}

TEST_F(M6502Test, STAIndirectXCanStoreARegisterIntoMemory) {
    mem[0xFFFC] = CPU::INS_STA_IND_X;
    mem[0xFFFD] = 0x10; // Zero page address
    cpu.X = 0x05; // X offset
    cpu.A = 0x99;

    // Set up the effective address in zero page
    mem[0x0015] = 0x00; // Low byte of effective address
    mem[0x0016] = 0x80; // High byte of effective address (effective address is 0x8000)

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 6);
    
    EXPECT_EQ(cyclesUsed, 6);
    
    EXPECT_EQ(mem[0x8000], 0x99); // Memory at effective address should now hold the value from the register
    VerifyUnchangedFlagsForLoad(cpu, cpuCopy);
}

TEST_F(M6502Test, STAIndirectYCanStoreARegisterIntoMemory) {
    mem[0xFFFC] = CPU::INS_STA_IND_Y;
    mem[0xFFFD] = 0x10; // Zero page address
    cpu.Y = 0x15; // Y offset
    cpu.A = 0x99;

    // Set up the effective address in zero page
    mem[0x0010] = 0xF0; // Low byte of effective address
    mem[0x0011] = 0x80; // High byte of effective address (effective address is 0x8000)

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 6);
    
    EXPECT_EQ(cyclesUsed, 6);
    
    EXPECT_EQ(mem[0x8105], 0x99); // Memory at effective address should now hold the value from the register
    VerifyUnchangedFlagsForLoad(cpu, cpuCopy);
}
