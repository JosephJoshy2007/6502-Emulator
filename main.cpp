#include "m6502.h"

int main() {
    
    CPU cpu;
    Memory mem;
    cpu.Reset(mem);
    mem[0xFFFC] = CPU::INS_JSR;
    mem[0xFFFD] = 0x42;
    mem[0xFFFE] = 0x42;
    mem[0x4242] = CPU::INS_LDA_IMM;
    mem[0x4243] = 0x32;
    
    cpu.Execute(mem, 8);
    return 0;

}