#include <gtest/gtest.h>
#include "m6502.h"

class M6502LogicTest : public ::testing::Test {
protected:
    Memory mem;
    CPU cpu;

    virtual void SetUp() override {
        cpu.Reset(mem);
    }

    virtual void TearDown() override {
    }
};

void VerifyUnchangedFlagsForLogic(const CPU& current, const CPU& original) {
    SCOPED_TRACE("Verifying unchanged status flags (C, B, D, I, V)");
    EXPECT_EQ(current.C, original.C);
    EXPECT_EQ(current.B, original.B);
    EXPECT_EQ(current.D, original.D);
    EXPECT_EQ(current.I, original.I);
    EXPECT_EQ(current.V, original.V);
}

void TestLogicalImmediate(CPU& cpu, Memory& mem, Byte opcode) {
    cpu.Reset(mem);
    cpu.PC = 0xFFFC;
    cpu.Z = cpu.N = true;
    cpu.A = 0x65;

    mem[0xFFFC] = opcode; 
    mem[0xFFFD] = 0x84; 
    
    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);
    
    EXPECT_EQ(cyclesUsed, 2);

    if (opcode == CPU::INS_AND_IMM) {
        EXPECT_EQ(cpu.A, 0x04);
        EXPECT_FALSE(cpu.Z);
        EXPECT_FALSE(cpu.N);
        VerifyUnchangedFlagsForLogic(cpu, cpuCopy);
    } else if (opcode == CPU::INS_EOR_IMM) {
        EXPECT_EQ(cpu.A, 0xE1);
        EXPECT_FALSE(cpu.Z);
        EXPECT_TRUE(cpu.N);
        VerifyUnchangedFlagsForLogic(cpu, cpuCopy);
    } else if (opcode == CPU::INS_ORA_IMM) {
        EXPECT_EQ(cpu.A, 0xE5);
        EXPECT_FALSE(cpu.Z);
        EXPECT_TRUE(cpu.N); // 0xE5 has bit 7 set -> N = true
        VerifyUnchangedFlagsForLogic(cpu, cpuCopy);
    }
}

void TestLogicalZeroPage(CPU& cpu, Memory& mem, Byte opcode) {
    cpu.Reset(mem);
    cpu.PC = 0xFFFC;
    cpu.Z = cpu.N = true;
    cpu.A = 0x65;

    mem[0xFFFC] = opcode; 
    mem[0xFFFD] = 0x10; 
    mem[0x0010] = 0x42;
    
    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 3);
    
    EXPECT_EQ(cyclesUsed, 3);
    
    if (opcode == CPU::INS_AND_ZP) {
        EXPECT_EQ(cpu.A, 0x40);
        EXPECT_FALSE(cpu.Z); // 0x40 != 0
        EXPECT_FALSE(cpu.N);
        VerifyUnchangedFlagsForLogic(cpu, cpuCopy);
    } else if (opcode == CPU::INS_EOR_ZP) {
        EXPECT_EQ(cpu.A, 0x27);
        EXPECT_FALSE(cpu.Z);
        EXPECT_FALSE(cpu.N);
        VerifyUnchangedFlagsForLogic(cpu, cpuCopy);
    } else if (opcode == CPU::INS_ORA_ZP) {
        EXPECT_EQ(cpu.A, 0x67);
        EXPECT_FALSE(cpu.Z);
        EXPECT_FALSE(cpu.N);
        VerifyUnchangedFlagsForLogic(cpu, cpuCopy);
    } else if (opcode == CPU::INS_BIT_ZP) {
        EXPECT_FALSE(cpu.Z); // 0x65 & 0x42 != 0
        EXPECT_FALSE(cpu.N); // bit 7 of 0x42 is 0
        EXPECT_TRUE(cpu.V);  // bit 6 of 0x42 is 1
        EXPECT_EQ(cpu.C, cpuCopy.C);
        EXPECT_EQ(cpu.B, cpuCopy.B);
        EXPECT_EQ(cpu.D, cpuCopy.D);
        EXPECT_EQ(cpu.I, cpuCopy.I);
    }
}

void TestLogicalZeroPageWithXOffset(CPU& cpu, Memory& mem, Byte opcode) {
    cpu.Reset(mem);
    cpu.PC = 0xFFFC;
    cpu.Z = cpu.N = true;
    cpu.A = 0x65;
    cpu.X = 0x05;

    mem[0xFFFC] = opcode; 
    mem[0xFFFD] = 0x80;
    mem[0x0085] = 0x42;
    
    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);
    
    EXPECT_EQ(cyclesUsed, 4);
    
    if (opcode == CPU::INS_AND_ZP_X) {
        EXPECT_EQ(cpu.A, 0x40);
        EXPECT_FALSE(cpu.Z); // 0x40 != 0
        EXPECT_FALSE(cpu.N);
        VerifyUnchangedFlagsForLogic(cpu, cpuCopy);
    } else if (opcode == CPU::INS_EOR_ZP_X) {
        EXPECT_EQ(cpu.A, 0x27);
        EXPECT_FALSE(cpu.Z);
        EXPECT_FALSE(cpu.N);
        VerifyUnchangedFlagsForLogic(cpu, cpuCopy);
    } else if (opcode == CPU::INS_ORA_ZP_X) {
        EXPECT_EQ(cpu.A, 0x67);
        EXPECT_FALSE(cpu.Z);
        EXPECT_FALSE(cpu.N);
        VerifyUnchangedFlagsForLogic(cpu, cpuCopy);
    }
}

void TestLogicalAbsolute(CPU& cpu, Memory& mem, Byte opcode) {
    cpu.Reset(mem);
    cpu.PC = 0xFFFC;
    cpu.Z = cpu.N = true;
    cpu.A = 0x65;

    mem[0xFFFC] = opcode; 
    mem[0xFFFD] = 0x00; 
    mem[0xFFFE] = 0x80; 
    mem[0x8000] = 0x42;
    
    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);
    
    EXPECT_EQ(cyclesUsed, 4);
    
    if (opcode == CPU::INS_AND_ABS) {
        EXPECT_EQ(cpu.A, 0x40);
        EXPECT_FALSE(cpu.Z);
        EXPECT_FALSE(cpu.N);
        VerifyUnchangedFlagsForLogic(cpu, cpuCopy);
    } else if (opcode == CPU::INS_EOR_ABS) {
        EXPECT_EQ(cpu.A, 0x27);
        EXPECT_FALSE(cpu.Z);
        EXPECT_FALSE(cpu.N);
        VerifyUnchangedFlagsForLogic(cpu, cpuCopy);
    } else if (opcode == CPU::INS_ORA_ABS) {
        EXPECT_EQ(cpu.A, 0x67);
        EXPECT_FALSE(cpu.Z);
        EXPECT_FALSE(cpu.N);
        VerifyUnchangedFlagsForLogic(cpu, cpuCopy);
    } else if (opcode == CPU::INS_BIT_ABS) {
        EXPECT_FALSE(cpu.Z);
        EXPECT_FALSE(cpu.N);
        EXPECT_TRUE(cpu.V);
        EXPECT_EQ(cpu.C, cpuCopy.C);
        EXPECT_EQ(cpu.B, cpuCopy.B);
        EXPECT_EQ(cpu.D, cpuCopy.D);
        EXPECT_EQ(cpu.I, cpuCopy.I);
    }
}

void TestLogicalAbsoluteWithXOffset(CPU& cpu, Memory& mem, Byte opcode) {
    cpu.Reset(mem);
    cpu.PC = 0xFFFC;
    cpu.Z = cpu.N = true;
    cpu.A = 0x65;
    cpu.X = 0x05;

    mem[0xFFFC] = opcode; 
    mem[0xFFFD] = 0x00; 
    mem[0xFFFE] = 0x80; 
    mem[0x8005] = 0x42;
    
    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);
    
    EXPECT_EQ(cyclesUsed, 4);
    
    if (opcode == CPU::INS_AND_ABS_X) {
        EXPECT_EQ(cpu.A, 0x40);
        EXPECT_FALSE(cpu.Z);
        EXPECT_FALSE(cpu.N);
        VerifyUnchangedFlagsForLogic(cpu, cpuCopy);
    } else if (opcode == CPU::INS_EOR_ABS_X) {
        EXPECT_EQ(cpu.A, 0x27);
        EXPECT_FALSE(cpu.Z);
        EXPECT_FALSE(cpu.N);
        VerifyUnchangedFlagsForLogic(cpu, cpuCopy);
    } else if (opcode == CPU::INS_ORA_ABS_X) {
        EXPECT_EQ(cpu.A, 0x67);
        EXPECT_FALSE(cpu.Z);
        EXPECT_FALSE(cpu.N);
        VerifyUnchangedFlagsForLogic(cpu, cpuCopy);
    }
}

void TestLogicalAbsoluteWithYOffset(CPU& cpu, Memory& mem, Byte opcode) {
    cpu.Reset(mem);
    cpu.PC = 0xFFFC;
    cpu.Z = cpu.N = true;
    cpu.A = 0x65;
    cpu.Y = 0x05;

    mem[0xFFFC] = opcode; 
    mem[0xFFFD] = 0x00; 
    mem[0xFFFE] = 0x80; 
    mem[0x8005] = 0x42;
    
    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);
    
    EXPECT_EQ(cyclesUsed, 4);
    
    if (opcode == CPU::INS_AND_ABS_Y) {
        EXPECT_EQ(cpu.A, 0x40);
        EXPECT_FALSE(cpu.Z);
        EXPECT_FALSE(cpu.N);
        VerifyUnchangedFlagsForLogic(cpu, cpuCopy);
    } else if (opcode == CPU::INS_EOR_ABS_Y) {
        EXPECT_EQ(cpu.A, 0x27);
        EXPECT_FALSE(cpu.Z);
        EXPECT_FALSE(cpu.N);
        VerifyUnchangedFlagsForLogic(cpu, cpuCopy);
    } else if (opcode == CPU::INS_ORA_ABS_Y) {
        EXPECT_EQ(cpu.A, 0x67);
        EXPECT_FALSE(cpu.Z);
        EXPECT_FALSE(cpu.N);
        VerifyUnchangedFlagsForLogic(cpu, cpuCopy);
    }
}

void TestLogicalAbsoluteWithXOffsetAndPageCrossing(CPU& cpu, Memory& mem, Byte opcode) {
    cpu.Reset(mem);
    cpu.PC = 0xFFFC;
    cpu.Z = cpu.N = true;
    cpu.A = 0x65;
    cpu.X = 0x15;

    mem[0xFFFC] = opcode; 
    mem[0xFFFD] = 0xF0; 
    mem[0xFFFE] = 0x80; 
    mem[0x8105] = 0x42;
    
    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 5);
    
    EXPECT_EQ(cyclesUsed, 5);
    
    if (opcode == CPU::INS_AND_ABS_X) {
        EXPECT_EQ(cpu.A, 0x40);
        EXPECT_FALSE(cpu.Z);
        EXPECT_FALSE(cpu.N);
        VerifyUnchangedFlagsForLogic(cpu, cpuCopy);
    } else if (opcode == CPU::INS_EOR_ABS_X) {
        EXPECT_EQ(cpu.A, 0x27);
        EXPECT_FALSE(cpu.Z);
        EXPECT_FALSE(cpu.N);
        VerifyUnchangedFlagsForLogic(cpu, cpuCopy);
    } else if (opcode == CPU::INS_ORA_ABS_X) {
        EXPECT_EQ(cpu.A, 0x67);
        EXPECT_FALSE(cpu.Z);
        EXPECT_FALSE(cpu.N);
        VerifyUnchangedFlagsForLogic(cpu, cpuCopy);
    }
}

void TestLogicalAbsoluteWithYOffsetAndPageCrossing(CPU& cpu, Memory& mem, Byte opcode) {
    cpu.Reset(mem);
    cpu.PC = 0xFFFC;
    cpu.Z = cpu.N = true;
    cpu.A = 0x65;
    cpu.Y = 0x15;

    mem[0xFFFC] = opcode; 
    mem[0xFFFD] = 0xF0; 
    mem[0xFFFE] = 0x80; 
    mem[0x8105] = 0x42;
    
    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 5);
    
    EXPECT_EQ(cyclesUsed, 5);
    
    if (opcode == CPU::INS_AND_ABS_Y) {
        EXPECT_EQ(cpu.A, 0x40);
        EXPECT_FALSE(cpu.Z);
        EXPECT_FALSE(cpu.N);
        VerifyUnchangedFlagsForLogic(cpu, cpuCopy);
    } else if (opcode == CPU::INS_EOR_ABS_Y) {
        EXPECT_EQ(cpu.A, 0x27);
        EXPECT_FALSE(cpu.Z);
        EXPECT_FALSE(cpu.N);
        VerifyUnchangedFlagsForLogic(cpu, cpuCopy);
    } else if (opcode == CPU::INS_ORA_ABS_Y) {
        EXPECT_EQ(cpu.A, 0x67);
        EXPECT_FALSE(cpu.Z);
        EXPECT_FALSE(cpu.N);
        VerifyUnchangedFlagsForLogic(cpu, cpuCopy);
    }
}

void TestLogicalIndirectX(CPU& cpu, Memory& mem, Byte opcode) {
    cpu.Reset(mem);
    cpu.PC = 0xFFFC;
    cpu.Z = cpu.N = true;
    cpu.A = 0x65; 
    cpu.X = 0x04; 

    mem[0xFFFC] = opcode; 
    mem[0xFFFD] = 0x10;
    mem[0x0014] = 0x00; 
    mem[0x0015] = 0x80; 
    mem[0x8000] = 0x42; 
    
    CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 6);

    EXPECT_EQ(cyclesUsed, 6);
    
    if (opcode == CPU::INS_AND_IND_X) {
        EXPECT_EQ(cpu.A, 0x40);
        EXPECT_FALSE(cpu.Z);
        EXPECT_FALSE(cpu.N);
        VerifyUnchangedFlagsForLogic(cpu, cpuCopy);
    } else if (opcode == CPU::INS_EOR_IND_X) {
        EXPECT_EQ(cpu.A, 0x27);
        EXPECT_FALSE(cpu.Z);
        EXPECT_FALSE(cpu.N);
        VerifyUnchangedFlagsForLogic(cpu, cpuCopy);
    } else if (opcode == CPU::INS_ORA_IND_X) {
        EXPECT_EQ(cpu.A, 0x67);
        EXPECT_FALSE(cpu.Z);
        EXPECT_FALSE(cpu.N);
        VerifyUnchangedFlagsForLogic(cpu, cpuCopy);
    }
}

void TestLogicalIndirectY(CPU& cpu, Memory& mem, Byte opcode) {
    cpu.Reset(mem);
    cpu.PC = 0xFFFC;
    cpu.Z = cpu.N = true;
    cpu.A = 0x65; 
    cpu.Y = 0x04; 

    mem[0xFFFC] = opcode; 
    mem[0xFFFD] = 0x10;
    // Pointer at $0010/$0011 -> $8000 + Y(4) -> $8004
    mem[0x0010] = 0x00; 
    mem[0x0011] = 0x80; 
    mem[0x8004] = 0x42; 
    
    CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 5); // 5 cycles without page boundary crossing

    EXPECT_EQ(cyclesUsed, 5);
    
    if (opcode == CPU::INS_AND_IND_Y) {
        EXPECT_EQ(cpu.A, 0x40);
        EXPECT_FALSE(cpu.Z);
        EXPECT_FALSE(cpu.N);
        VerifyUnchangedFlagsForLogic(cpu, cpuCopy);
    } else if (opcode == CPU::INS_EOR_IND_Y) {
        EXPECT_EQ(cpu.A, 0x27);
        EXPECT_FALSE(cpu.Z);
        EXPECT_FALSE(cpu.N);
        VerifyUnchangedFlagsForLogic(cpu, cpuCopy);
    } else if (opcode == CPU::INS_ORA_IND_Y) {
        EXPECT_EQ(cpu.A, 0x67);
        EXPECT_FALSE(cpu.Z);
        EXPECT_FALSE(cpu.N);
        VerifyUnchangedFlagsForLogic(cpu, cpuCopy);
    }
}

void TestLogicalIndirectYWithPageCrossing(CPU& cpu, Memory& mem, Byte opcode) {
    cpu.Reset(mem);
    cpu.PC = 0xFFFC;
    cpu.Z = cpu.N = true;
    cpu.A = 0x65;
    cpu.Y = 0x15; 

    mem[0xFFFC] = opcode; 
    mem[0xFFFD] = 0x10; 
    mem[0x0010] = 0xF0; 
    mem[0x0011] = 0x80; 
    mem[0x8105] = 0x42; // Same target test byte
    
    CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 6); // 5 + 1 extra cycle for page cross

    EXPECT_EQ(cyclesUsed, 6);
    
    if (opcode == CPU::INS_AND_IND_Y) {
        EXPECT_EQ(cpu.A, 0x40);
        EXPECT_FALSE(cpu.Z);
        EXPECT_FALSE(cpu.N);
        VerifyUnchangedFlagsForLogic(cpu, cpuCopy);
    } else if (opcode == CPU::INS_EOR_IND_Y) {
        EXPECT_EQ(cpu.A, 0x27);
        EXPECT_FALSE(cpu.Z);
        EXPECT_FALSE(cpu.N);
        VerifyUnchangedFlagsForLogic(cpu, cpuCopy);
    } else if (opcode == CPU::INS_ORA_IND_Y) {
        EXPECT_EQ(cpu.A, 0x67);
        EXPECT_FALSE(cpu.Z);
        EXPECT_FALSE(cpu.N);
        VerifyUnchangedFlagsForLogic(cpu, cpuCopy);
    }
}

TEST_F(M6502LogicTest, TestLogicalImmediateForAll) {
    TestLogicalImmediate(cpu, mem, CPU::INS_AND_IMM);
    TestLogicalImmediate(cpu, mem, CPU::INS_EOR_IMM);
    TestLogicalImmediate(cpu, mem, CPU::INS_ORA_IMM);
}

TEST_F(M6502LogicTest, TestLogicalZeroPageForAll) {
    TestLogicalZeroPage(cpu, mem, CPU::INS_AND_ZP);
    TestLogicalZeroPage(cpu, mem, CPU::INS_EOR_ZP);
    TestLogicalZeroPage(cpu, mem, CPU::INS_ORA_ZP);
    TestLogicalZeroPage(cpu, mem, CPU::INS_BIT_ZP);
}

TEST_F(M6502LogicTest, TestLogicalZeroPageWithXOffset) {
    TestLogicalZeroPageWithXOffset(cpu, mem, CPU::INS_AND_ZP_X);
    TestLogicalZeroPageWithXOffset(cpu, mem, CPU::INS_EOR_ZP_X);
    TestLogicalZeroPageWithXOffset(cpu, mem, CPU::INS_ORA_ZP_X);
}

TEST_F(M6502LogicTest, TestLogicalAbsolute) {
    TestLogicalAbsolute(cpu, mem, CPU::INS_AND_ABS);
    TestLogicalAbsolute(cpu, mem, CPU::INS_EOR_ABS);
    TestLogicalAbsolute(cpu, mem, CPU::INS_ORA_ABS);
    TestLogicalAbsolute(cpu, mem, CPU::INS_BIT_ABS);
}

TEST_F(M6502LogicTest, TestLogicalAbsoluteWithXOffset){
    TestLogicalAbsoluteWithXOffset(cpu, mem, CPU::INS_AND_ABS_X);
    TestLogicalAbsoluteWithXOffset(cpu, mem, CPU::INS_EOR_ABS_X);
    TestLogicalAbsoluteWithXOffset(cpu, mem, CPU::INS_ORA_ABS_X);
}

TEST_F(M6502LogicTest, TestLogicalAbsoluteWithYOffset){
    TestLogicalAbsoluteWithYOffset(cpu, mem, CPU::INS_AND_ABS_Y);
    TestLogicalAbsoluteWithYOffset(cpu, mem, CPU::INS_EOR_ABS_Y);
    TestLogicalAbsoluteWithYOffset(cpu, mem, CPU::INS_ORA_ABS_Y);
}

TEST_F(M6502LogicTest, TestLogicalAbsoluteWithXOffsetAndPageCrossing){
    TestLogicalAbsoluteWithXOffsetAndPageCrossing(cpu, mem, CPU::INS_AND_ABS_X);
    TestLogicalAbsoluteWithXOffsetAndPageCrossing(cpu, mem, CPU::INS_EOR_ABS_X);
    TestLogicalAbsoluteWithXOffsetAndPageCrossing(cpu, mem, CPU::INS_ORA_ABS_X);
}

TEST_F(M6502LogicTest, TestLogicalAbsoluteWithYOffsetAndPageCrossing){
    TestLogicalAbsoluteWithYOffsetAndPageCrossing(cpu, mem, CPU::INS_AND_ABS_Y);
    TestLogicalAbsoluteWithYOffsetAndPageCrossing(cpu, mem, CPU::INS_EOR_ABS_Y);
    TestLogicalAbsoluteWithYOffsetAndPageCrossing(cpu, mem, CPU::INS_ORA_ABS_Y);
}

TEST_F(M6502LogicTest, TestLogicalIndirectX) {
    TestLogicalIndirectX(cpu, mem, CPU::INS_AND_IND_X);
    TestLogicalIndirectX(cpu, mem, CPU::INS_EOR_IND_X);
    TestLogicalIndirectX(cpu, mem, CPU::INS_ORA_IND_X);
}

TEST_F(M6502LogicTest, TestLogicalIndirectY) {
    TestLogicalIndirectY(cpu, mem, CPU::INS_AND_IND_Y);
    TestLogicalIndirectY(cpu, mem, CPU::INS_EOR_IND_Y);
    TestLogicalIndirectY(cpu, mem, CPU::INS_ORA_IND_Y);
}

TEST_F(M6502LogicTest, TestLogicalIndirectYWithPageCrossing) {
    TestLogicalIndirectYWithPageCrossing(cpu, mem, CPU::INS_AND_IND_Y);
    TestLogicalIndirectYWithPageCrossing(cpu, mem, CPU::INS_EOR_IND_Y);
    TestLogicalIndirectYWithPageCrossing(cpu, mem, CPU::INS_ORA_IND_Y);
}