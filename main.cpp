#include "m6502.h"
#include <stdio.h>
#include <windows.h>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

fs::path GetExecutableDirectory() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    return fs::path(buffer).parent_path();
}

fs::path FindBinaryFile(const std::string& relativeTarget) {
    fs::path current = GetExecutableDirectory();

    // Traverse upwards (up to 5 levels) looking for the target file
    for (int i = 0; i < 5; ++i) {
        fs::path candidate = current / relativeTarget;
        if (fs::exists(candidate)) {
            return candidate;
        }
        if (!current.has_parent_path()) break;
        current = current.parent_path();
    }
    return {};
}

int main() {
    
    CPU cpu;
    Memory mem;
    cpu.Reset(mem);

    fs::path binPath = FindBinaryFile("compile/program.bin");

    if (!cpu.LoadProgram(binPath.string(), mem, cpu)) {
        return 1;
    }
    
    cpu.UseResetVector(mem);
    cpu.Execute(mem, 500);

    return 0;

}