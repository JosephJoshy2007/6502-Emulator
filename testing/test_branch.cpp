#include <gtest/gtest.h>
#include <functional>
#include "m6502.h"

class M6502BranchTest : public ::testing::Test {
protected:
    Memory mem;
    CPU cpu;

    virtual void SetUp() override {
        cpu.Reset(mem);
    }

    virtual void TearDown() override {
    }
};

void VerifyUnchangedFlagsForBranch(const CPU& current, const CPU& original) {
    SCOPED_TRACE("Verifying unchanged status flags (C, B, D, I, V)");
    EXPECT_EQ(current.C, original.C);
    EXPECT_EQ(current.B, original.B);
    EXPECT_EQ(current.D, original.D);
    EXPECT_EQ(current.I, original.I);
    EXPECT_EQ(current.V, original.V);
    EXPECT_EQ(current.N, original.N);
    EXPECT_EQ(current.Z, original.Z);
}

TEST_F(M6502BranchTest, BCCBranchesMaxForwardOffset) {
    cpu.PC = 0x8000;
    cpu.C = false;

    mem[0x8000] = CPU::INS_BCC;
    mem[0x8001] = 0x7F; // +127: (0x8000 + 2) + 127 = 0x8081

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 3);

    EXPECT_EQ(cpu.PC, 0x8081);
    EXPECT_EQ(cyclesUsed, 3);
    VerifyUnchangedFlagsForBranch(cpu, cpuCopy);
}

// Maximum Negative Offset (-128 / 0x80)
TEST_F(M6502BranchTest, BCCBranchesMaxBackwardOffset) {
    cpu.PC = 0x8080;
    cpu.C = false;

    mem[0x8080] = CPU::INS_BCC;
    mem[0x8081] = 0x80; // -128: (0x8080 + 2) - 128 = 0x8002

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 3);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 3);
    VerifyUnchangedFlagsForBranch(cpu, cpuCopy);
}

// Zero Offset (+0 / 0x00) - Still takes 3 cycles because branch condition is met
TEST_F(M6502BranchTest, BCCBranchesZeroOffset) {
    cpu.PC = 0x8000;
    cpu.C = false;

    mem[0x8000] = CPU::INS_BCC;
    mem[0x8001] = 0x00; // (0x8000 + 2) + 0 = 0x8002

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 3);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 3);
    VerifyUnchangedFlagsForBranch(cpu, cpuCopy);
}

// Forward Page Crossing (4 Cycles)
TEST_F(M6502BranchTest, BCCPageCrossingForward) {
    cpu.PC = 0x80FD;
    cpu.C = false;

    mem[0x80FD] = CPU::INS_BCC;
    mem[0x80FE] = 0x02; // Base PC = 0x80FF; 0x80FF + 2 = 0x8101 (page 0x80 -> 0x81)

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x8101);
    EXPECT_EQ(cyclesUsed, 4);
    VerifyUnchangedFlagsForBranch(cpu, cpuCopy);
}

// Backward Page Crossing (4 Cycles)
TEST_F(M6502BranchTest, BCCPageCrossingBackward) {
    cpu.PC = 0x8100;
    cpu.C = false;

    mem[0x8100] = CPU::INS_BCC;
    mem[0x8101] = 0xFA; // -6 in signed 8-bit; Base PC = 0x8102; 0x8102 - 6 = 0x80FC

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x80FC);
    EXPECT_EQ(cyclesUsed, 4);
    VerifyUnchangedFlagsForBranch(cpu, cpuCopy);
}

// Boundary Exact Fit (Does NOT cross page boundary, 3 cycles)
// Lands on 0x8100 when base PC is already on 0x8100
TEST_F(M6502BranchTest, BCCDoesNotCrossPageWhenLandingOnBoundary) {
    cpu.PC = 0x80FE;
    cpu.C = false;

    mem[0x80FE] = CPU::INS_BCC;
    mem[0x80FF] = 0x00; // Base PC = 0x8100; 0x8100 + 0 = 0x8100 (both in page 0x81)

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 3);

    EXPECT_EQ(cpu.PC, 0x8100);
    EXPECT_EQ(cyclesUsed, 3);
    VerifyUnchangedFlagsForBranch(cpu, cpuCopy);
}

TEST_F(M6502BranchTest, BCCBranchesWhenCarryIsClear) {

    cpu.Reset(mem);
    cpu.PC = 0x8000;
    cpu.C = false;

    mem[0x8000] = CPU::INS_BCC;
    mem[0x8001] = 0x02;
    mem[0x8004] = CPU::INS_LDA_IMM;
    mem[0x8005] = 0x42;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(mem[0x8005], cpu.A);
    EXPECT_EQ(cyclesUsed, 5);

    VerifyUnchangedFlagsForBranch(cpu, cpuCopy);
    
}

TEST_F(M6502BranchTest, BCCDoesNotBranchWhenCarryIsSet) {
    cpu.Reset(mem);
    cpu.PC = 0x8000;
    cpu.C = true;

    mem[0x8000] = CPU::INS_BCC;
    mem[0x8001] = 0x02;

    const CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 2);
    VerifyUnchangedFlagsForBranch(cpu, cpuCopy);
}

TEST_F(M6502BranchTest, BCCBranchesBackwardWithNegativeOffset) {
    cpu.Reset(mem);
    cpu.PC = 0x8006;
    cpu.C = false;

    mem[0x8006] = CPU::INS_BCC;
    mem[0x8007] = 0xFB; // -5 in two's complement

    s32 cyclesUsed = cpu.Execute(mem, 3);

    EXPECT_EQ(cpu.PC, 0x8003);
    EXPECT_EQ(cyclesUsed, 3);
}

TEST_F(M6502BranchTest, BCCAddsExtraCycleWhenCrossingPageBoundaryForward) {
    cpu.Reset(mem);
    cpu.PC = 0x80FD;
    cpu.C = false;

    mem[0x80FD] = CPU::INS_BCC;
    mem[0x80FE] = 0x02; // PC base after fetch is 0x80FF; 0x80FF + 2 = 0x8101

    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x8101);
    EXPECT_EQ(cyclesUsed, 4);
}

TEST_F(M6502BranchTest, BCCAddsExtraCycleWhenCrossingPageBoundaryBackward) {
    cpu.Reset(mem);
    cpu.PC = 0x8100;
    cpu.C = false;

    mem[0x8100] = CPU::INS_BCC;
    mem[0x8101] = 0xFA; // -6 in two's complement; PC base is 0x8102; 0x8102 - 6 = 0x80FC

    s32 cyclesUsed = cpu.Execute(mem, 4);

    EXPECT_EQ(cpu.PC, 0x80FC);
    EXPECT_EQ(cyclesUsed, 4);
}

// ---------------------------------------------------------------------------
// BEQ (Branch if Equal / Zero Set)
// ---------------------------------------------------------------------------
TEST_F(M6502BranchTest, BEQBranchesWhenZeroIsSet) {
    cpu.Reset(mem);
    cpu.PC = 0x8000;
    cpu.Z = true;

    mem[0x8000] = CPU::INS_BEQ;
    mem[0x8001] = 0x05;

    s32 cyclesUsed = cpu.Execute(mem, 3);

    EXPECT_EQ(cpu.PC, 0x8007); // 0x8002 + 5
    EXPECT_EQ(cyclesUsed, 3);
}

TEST_F(M6502BranchTest, BEQDoesNotBranchWhenZeroIsClear) {
    cpu.Reset(mem);
    cpu.PC = 0x8000;
    cpu.Z = false;

    mem[0x8000] = CPU::INS_BEQ;
    mem[0x8001] = 0x05;

    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 2);
}

// ---------------------------------------------------------------------------
// BNE (Branch if Not Equal / Zero Clear)
// ---------------------------------------------------------------------------
TEST_F(M6502BranchTest, BNEBranchesWhenZeroIsClear) {
    cpu.Reset(mem);
    cpu.PC = 0x8000;
    cpu.Z = false;

    mem[0x8000] = CPU::INS_BNE;
    mem[0x8001] = 0x05;

    s32 cyclesUsed = cpu.Execute(mem, 3);

    EXPECT_EQ(cpu.PC, 0x8007);
    EXPECT_EQ(cyclesUsed, 3);
}

TEST_F(M6502BranchTest, BNEDoesNotBranchWhenZeroIsSet) {
    cpu.Reset(mem);
    cpu.PC = 0x8000;
    cpu.Z = true;

    mem[0x8000] = CPU::INS_BNE;
    mem[0x8001] = 0x05;

    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 2);
}

// ---------------------------------------------------------------------------
// BCS (Branch if Carry Set)
// ---------------------------------------------------------------------------
TEST_F(M6502BranchTest, BCSBranchesWhenCarryIsSet) {
    cpu.Reset(mem);
    cpu.PC = 0x8000;
    cpu.C = true;

    mem[0x8000] = CPU::INS_BCS;
    mem[0x8001] = 0x05;

    s32 cyclesUsed = cpu.Execute(mem, 3);

    EXPECT_EQ(cpu.PC, 0x8007);
    EXPECT_EQ(cyclesUsed, 3);
}

TEST_F(M6502BranchTest, BCSDoesNotBranchWhenCarryIsClear) {
    cpu.Reset(mem);
    cpu.PC = 0x8000;
    cpu.C = false;

    mem[0x8000] = CPU::INS_BCS;
    mem[0x8001] = 0x05;

    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 2);
}

// ---------------------------------------------------------------------------
// BMI (Branch if Minus / Negative Set)
// ---------------------------------------------------------------------------
TEST_F(M6502BranchTest, BMIBranchesWhenNegativeIsSet) {
    cpu.Reset(mem);
    cpu.PC = 0x8000;
    cpu.N = true;

    mem[0x8000] = CPU::INS_BMI;
    mem[0x8001] = 0x05;

    s32 cyclesUsed = cpu.Execute(mem, 3);

    EXPECT_EQ(cpu.PC, 0x8007);
    EXPECT_EQ(cyclesUsed, 3);
}

TEST_F(M6502BranchTest, BMIDoesNotBranchWhenNegativeIsClear) {
    cpu.Reset(mem);
    cpu.PC = 0x8000;
    cpu.N = false;

    mem[0x8000] = CPU::INS_BMI;
    mem[0x8001] = 0x05;

    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 2);
}

// ---------------------------------------------------------------------------
// BPL (Branch if Positive / Negative Clear)
// ---------------------------------------------------------------------------
TEST_F(M6502BranchTest, BPLBranchesWhenNegativeIsClear) {
    cpu.Reset(mem);
    cpu.PC = 0x8000;
    cpu.N = false;

    mem[0x8000] = CPU::INS_BPL;
    mem[0x8001] = 0x05;

    s32 cyclesUsed = cpu.Execute(mem, 3);

    EXPECT_EQ(cpu.PC, 0x8007);
    EXPECT_EQ(cyclesUsed, 3);
}

TEST_F(M6502BranchTest, BPLDoesNotBranchWhenNegativeIsSet) {
    cpu.Reset(mem);
    cpu.PC = 0x8000;
    cpu.N = true;

    mem[0x8000] = CPU::INS_BPL;
    mem[0x8001] = 0x05;

    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 2);
}

// ---------------------------------------------------------------------------
// BVS (Branch if Overflow Set)
// ---------------------------------------------------------------------------
TEST_F(M6502BranchTest, BVSBranchesWhenOverflowIsSet) {
    cpu.Reset(mem);
    cpu.PC = 0x8000;
    cpu.V = true;

    mem[0x8000] = CPU::INS_BVS;
    mem[0x8001] = 0x05;

    s32 cyclesUsed = cpu.Execute(mem, 3);

    EXPECT_EQ(cpu.PC, 0x8007);
    EXPECT_EQ(cyclesUsed, 3);
}

TEST_F(M6502BranchTest, BVSDoesNotBranchWhenOverflowIsClear) {
    cpu.Reset(mem);
    cpu.PC = 0x8000;
    cpu.V = false;

    mem[0x8000] = CPU::INS_BVS;
    mem[0x8001] = 0x05;

    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 2);
}

// ---------------------------------------------------------------------------
// BVC (Branch if Overflow Clear)
// ---------------------------------------------------------------------------
TEST_F(M6502BranchTest, BVCBranchesWhenOverflowIsClear) {
    cpu.Reset(mem);
    cpu.PC = 0x8000;
    cpu.V = false;

    mem[0x8000] = CPU::INS_BVC;
    mem[0x8001] = 0x05;

    s32 cyclesUsed = cpu.Execute(mem, 3);

    EXPECT_EQ(cpu.PC, 0x8007);
    EXPECT_EQ(cyclesUsed, 3);
}

TEST_F(M6502BranchTest, BVCDoesNotBranchWhenOverflowIsSet) {
    cpu.Reset(mem);
    cpu.PC = 0x8000;
    cpu.V = true;

    mem[0x8000] = CPU::INS_BVC;
    mem[0x8001] = 0x05;

    s32 cyclesUsed = cpu.Execute(mem, 2);

    EXPECT_EQ(cpu.PC, 0x8002);
    EXPECT_EQ(cyclesUsed, 2);
}