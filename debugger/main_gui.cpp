#include <iostream>
#include <fstream>
#include <chrono>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "m6502.h"
#include "Debugger.h"
#include "DebuggerUI.h"

static void GlfwErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

int main(int argc, char** argv) {
    glfwSetErrorCallback(GlfwErrorCallback);
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return 1;
    }

    // GL 3.3 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(1440, 900, "MOS 6502 Emulator & Debugger", nullptr, nullptr);
    if (!window) {
        // Fallback to default context if 3.3 core profile fails
        glfwDefaultWindowHints();
        window = glfwCreateWindow(1440, 900, "MOS 6502 Emulator & Debugger", nullptr, nullptr);
        if (!window) {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return 1;
        }
        glsl_version = "#version 120";
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Theme & style
    DebuggerUI::SetupTheme();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Initialize 6502 CPU, Memory, and Debugger
    CPU cpu;
    Memory memory;
    cpu.Reset(memory);

    Debugger dbg(cpu, memory);

    // Load initial program if available
    std::string defaultBin = "compile/program.bin";
    std::ifstream testFile(defaultBin, std::ios::binary);
    if (testFile.is_open()) {
        testFile.close();
        if (dbg.LoadBinary(defaultBin, 0x8000)) {
            // Set reset vector to 0x8000 so Reset points to our loaded binary
            memory[0xFFFC] = 0x00;
            memory[0xFFFD] = 0x80;
            dbg.Reset();
        }
    } else {
        // Fallback: simple demo program in memory at $8000
        // LDA #$42; LDX #$05; STA $0200; INX; JMP $8004
        u16 addr = 0x8000;
        memory[addr++] = 0xA9; memory[addr++] = 0x42;       // LDA #$42
        memory[addr++] = 0xA2; memory[addr++] = 0x05;       // LDX #$05
        memory[addr++] = 0x8D; memory[addr++] = 0x00; memory[addr++] = 0x02; // STA $0200
        memory[addr++] = 0xE8;                               // INX
        memory[addr++] = 0x4C; memory[addr++] = 0x04; memory[addr++] = 0x80; // JMP $8004

        memory[0xFFFC] = 0x00;
        memory[0xFFFD] = 0x80;
        dbg.Reset();
    }

    auto lastTime = std::chrono::high_resolution_clock::now();

    // Main render loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        // Debugger continuous update
        dbg.Update(deltaTime);

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Render Debugger UI
        DebuggerUI::Render(dbg);

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.09f, 0.09f, 0.11f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
