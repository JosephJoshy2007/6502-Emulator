#include "DebuggerUI.h"
#include "Disassembler.h"
#include "imgui.h"
#include <cstdio>
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace {

char g_LoadPathBuffer[256] = "compile/program.bin";
char g_MemJumpBuffer[16] = "8000";
char g_DisasmJumpBuffer[16] = "8000";
char g_BreakpointInputBuffer[16] = "8041";

bool g_FollowPC = true;
Word g_SelectedMemAddress = 0x8000;
bool g_MemEditing = false;
Word g_MemEditAddr = 0;
char g_MemEditValBuffer[4] = "";

int g_StepCount = 10;
int g_CycleBudget = 100;

// Helper to parse hex string
Word ParseHex(const char* str, Word defaultVal = 0) {
    if (!str || !*str) return defaultVal;
    if (str[0] == '$') str++;
    else if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) str += 2;
    try {
        return static_cast<Word>(std::stoul(str, nullptr, 16) & 0xFFFF);
    } catch (...) {
        return defaultVal;
    }
}

} // anonymous namespace

void DebuggerUI::SetupTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;

    // Dark charcoal hardware-debugger palette
    colors[ImGuiCol_Text]                  = ImVec4(0.92f, 0.94f, 0.96f, 1.00f);
    colors[ImGuiCol_TextDisabled]          = ImVec4(0.50f, 0.54f, 0.58f, 1.00f);
    colors[ImGuiCol_WindowBg]              = ImVec4(0.11f, 0.12f, 0.14f, 1.00f);
    colors[ImGuiCol_ChildBg]               = ImVec4(0.13f, 0.14f, 0.16f, 1.00f);
    colors[ImGuiCol_PopupBg]               = ImVec4(0.14f, 0.15f, 0.18f, 0.98f);
    colors[ImGuiCol_Border]                = ImVec4(0.24f, 0.27f, 0.32f, 1.00f);
    colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]               = ImVec4(0.17f, 0.19f, 0.22f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.22f, 0.25f, 0.30f, 1.00f);
    colors[ImGuiCol_FrameBgActive]         = ImVec4(0.26f, 0.30f, 0.36f, 1.00f);
    colors[ImGuiCol_TitleBg]               = ImVec4(0.10f, 0.11f, 0.13f, 1.00f);
    colors[ImGuiCol_TitleBgActive]         = ImVec4(0.15f, 0.17f, 0.21f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.10f, 0.11f, 0.13f, 0.60f);
    colors[ImGuiCol_MenuBarBg]             = ImVec4(0.14f, 0.15f, 0.18f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.12f, 0.13f, 0.15f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.24f, 0.27f, 0.32f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.32f, 0.36f, 0.42f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.40f, 0.46f, 0.54f, 1.00f);
    colors[ImGuiCol_CheckMark]             = ImVec4(0.22f, 0.85f, 0.45f, 1.00f);
    colors[ImGuiCol_SliderGrab]            = ImVec4(0.25f, 0.58f, 0.95f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.35f, 0.68f, 1.00f, 1.00f);
    colors[ImGuiCol_Button]                = ImVec4(0.20f, 0.23f, 0.28f, 1.00f);
    colors[ImGuiCol_ButtonHovered]         = ImVec4(0.28f, 0.32f, 0.40f, 1.00f);
    colors[ImGuiCol_ButtonActive]          = ImVec4(0.36f, 0.42f, 0.52f, 1.00f);
    colors[ImGuiCol_Header]                = ImVec4(0.18f, 0.22f, 0.28f, 1.00f);
    colors[ImGuiCol_HeaderHovered]         = ImVec4(0.25f, 0.30f, 0.38f, 1.00f);
    colors[ImGuiCol_HeaderActive]          = ImVec4(0.30f, 0.38f, 0.48f, 1.00f);
    colors[ImGuiCol_Separator]             = ImVec4(0.22f, 0.25f, 0.30f, 1.00f);
    colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.30f, 0.35f, 0.44f, 1.00f);
    colors[ImGuiCol_SeparatorActive]       = ImVec4(0.40f, 0.48f, 0.60f, 1.00f);
    colors[ImGuiCol_Tab]                   = ImVec4(0.14f, 0.16f, 0.19f, 1.00f);
    colors[ImGuiCol_TabHovered]            = ImVec4(0.24f, 0.28f, 0.35f, 1.00f);
    colors[ImGuiCol_TabActive]             = ImVec4(0.20f, 0.24f, 0.30f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]         = ImVec4(0.16f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_TableBorderStrong]     = ImVec4(0.26f, 0.30f, 0.36f, 1.00f);
    colors[ImGuiCol_TableBorderLight]      = ImVec4(0.20f, 0.23f, 0.28f, 1.00f);
    colors[ImGuiCol_TableRowBg]            = ImVec4(0.12f, 0.13f, 0.15f, 1.00f);
    colors[ImGuiCol_TableRowBgAlt]         = ImVec4(0.14f, 0.15f, 0.18f, 1.00f);
}

void DebuggerUI::Render(Debugger& dbg) {
    // Fill the viewport with a main docking/layout window
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | 
                                   ImGuiWindowFlags_NoMove | 
                                   ImGuiWindowFlags_NoResize | 
                                   ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
    ImGui::Begin("6502 Debugger", nullptr, windowFlags);
    ImGui::PopStyleVar();

    RenderTopBar(dbg);
    ImGui::Separator();
    ImGui::Spacing();

    // 2-column main area: Left Column (Controls, Registers, Flags, Disassembly)
    //                      Right Column (Memory Viewer, Stack Viewer, Stats & Breakpoints)
    float totalWidth = ImGui::GetContentRegionAvail().x;
    float leftColWidth = totalWidth * 0.48f;
    float rightColWidth = totalWidth - leftColWidth - 10.0f;

    // LEFT COLUMN
    ImGui::BeginChild("LeftColumn", ImVec2(leftColWidth, 0), false);
    {
        RenderRegistersAndFlags(dbg);
        ImGui::Spacing();
        RenderExecutionControls(dbg);
        ImGui::Spacing();
        RenderDisassembly(dbg);
    }
    ImGui::EndChild();

    ImGui::SameLine();

    // RIGHT COLUMN
    ImGui::BeginChild("RightColumn", ImVec2(rightColWidth, 0), false);
    {
        RenderMemoryViewer(dbg);
        ImGui::Spacing();
        
        float subHeight = ImGui::GetContentRegionAvail().y;
        float stackHeight = subHeight * 0.55f;
        
        ImGui::BeginChild("StackSection", ImVec2(0, stackHeight), true);
        RenderStackViewer(dbg);
        ImGui::EndChild();

        ImGui::Spacing();

        ImGui::BeginChild("StatsSection", ImVec2(0, 0), true);
        RenderBreakpointsAndStats(dbg);
        ImGui::EndChild();
    }
    ImGui::EndChild();

    ImGui::End();
}

void DebuggerUI::RenderTopBar(Debugger& dbg) {
    ImGui::TextColored(ImVec4(0.25f, 0.75f, 1.0f, 1.0f), "MOS 6502 INTERACTIVE DEBUGGER");
    ImGui::SameLine();
    ImGui::TextDisabled("|");
    ImGui::SameLine();

    // Status indicator badge
    if (dbg.state == DebuggerState::Running) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.22f, 0.90f, 0.45f, 1.0f));
        ImGui::Text("[ RUNNING ]");
        ImGui::PopStyleColor();
    } else if (dbg.hitBreakpoint) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        ImGui::Text("[ BREAKPOINT AT 0x%04X ]", dbg.breakpointHitAddress);
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.2f, 1.0f));
        ImGui::Text("[ PAUSED ]");
        ImGui::PopStyleColor();
    }

    ImGui::SameLine(ImGui::GetWindowWidth() - 480.0f);
    ImGui::SetNextItemWidth(260.0f);
    ImGui::InputText("##LoadPath", g_LoadPathBuffer, sizeof(g_LoadPathBuffer));
    ImGui::SameLine();
    if (ImGui::Button("Load Binary")) {
        if (dbg.LoadBinary(g_LoadPathBuffer)) {
            g_FollowPC = true;
        }
    }
}

void DebuggerUI::RenderRegistersAndFlags(Debugger& dbg) {
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.18f, 0.22f, 0.28f, 1.0f));
    if (ImGui::CollapsingHeader("CPU REGISTERS & PROCESSOR STATUS", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::PopStyleColor();

        // Registers row with change highlighting
        auto RenderRegBox = [&](const char* name, int val, int prevVal, int digits) {
            bool changed = (val != prevVal);
            ImGui::BeginGroup();
            ImGui::TextDisabled("%s", name);
            if (changed) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.98f, 0.85f, 0.25f, 1.0f)); // Gold highlight on change
            } else {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
            }
            if (digits == 4) {
                ImGui::Text("0x%04X", val);
                ImGui::TextDisabled("(%d)", val);
            } else {
                ImGui::Text("0x%02X", val);
                ImGui::TextDisabled("(%d)", val);
            }
            ImGui::PopStyleColor();
            ImGui::EndGroup();
        };

        ImGui::Columns(6, "RegColumns", false);
        RenderRegBox("PC", dbg.cpu.PC, dbg.prevPC, 4);
        ImGui::NextColumn();
        RenderRegBox("A", dbg.cpu.A, dbg.prevA, 2);
        ImGui::NextColumn();
        RenderRegBox("X", dbg.cpu.X, dbg.prevX, 2);
        ImGui::NextColumn();
        RenderRegBox("Y", dbg.cpu.Y, dbg.prevY, 2);
        ImGui::NextColumn();
        RenderRegBox("SP", dbg.cpu.SP, dbg.prevSP, 2);
        ImGui::NextColumn();
        Byte p = dbg.cpu.GetStatusRegister();
        RenderRegBox("P", p, dbg.prevP, 2);
        ImGui::Columns(1);

        ImGui::Separator();
        ImGui::Spacing();

        // 6502 Status Flags Display: N, V, B, D, I, Z, C
        ImGui::Text("Status Flags:");
        ImGui::SameLine();

        auto RenderFlag = [](const char* label, bool val) {
            ImVec4 activeBg = ImVec4(0.15f, 0.65f, 0.30f, 1.0f);
            ImVec4 inactiveBg = ImVec4(0.18f, 0.20f, 0.23f, 1.0f);
            ImVec4 activeTxt = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            ImVec4 inactiveTxt = ImVec4(0.45f, 0.48f, 0.52f, 1.0f);

            ImGui::PushStyleColor(ImGuiCol_Button, val ? activeBg : inactiveBg);
            ImGui::PushStyleColor(ImGuiCol_Text, val ? activeTxt : inactiveTxt);
            ImGui::Button(label, ImVec2(32, 24));
            ImGui::PopStyleColor(2);
            ImGui::SameLine();
        };

        RenderFlag("N", dbg.cpu.N != 0);
        RenderFlag("V", dbg.cpu.V != 0);
        RenderFlag("B", (p & 0x10) != 0);
        RenderFlag("D", dbg.cpu.D != 0);
        RenderFlag("I", dbg.cpu.I != 0);
        RenderFlag("Z", dbg.cpu.Z != 0);
        RenderFlag("C", dbg.cpu.C != 0);

        ImGui::NewLine();
    } else {
        ImGui::PopStyleColor();
    }
}

void DebuggerUI::RenderExecutionControls(Debugger& dbg) {
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.18f, 0.22f, 0.28f, 1.0f));
    if (ImGui::CollapsingHeader("EXECUTION CONTROLS", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::PopStyleColor();

        // Primary Control Buttons
        if (dbg.state == DebuggerState::Running) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.65f, 0.15f, 1.0f));
            if (ImGui::Button(" PAUSE ", ImVec2(80, 30))) {
                dbg.Pause();
            }
            ImGui::PopStyleColor();
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.70f, 0.30f, 1.0f));
            if (ImGui::Button("  RUN  ", ImVec2(80, 30))) {
                dbg.Run();
            }
            ImGui::PopStyleColor();
        }

        ImGui::SameLine();
        if (ImGui::Button(" STEP ", ImVec2(80, 30))) {
            dbg.Step();
        }

        ImGui::SameLine();
        if (ImGui::Button("STEP OVER", ImVec2(90, 30))) {
            dbg.StepOver();
        }

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.65f, 0.20f, 0.20f, 1.0f));
        if (ImGui::Button(" RESET ", ImVec2(80, 30))) {
            dbg.Reset();
        }
        ImGui::PopStyleColor();

        ImGui::Spacing();

        // Speed Slider & Batch Stepping
        ImGui::SetNextItemWidth(180.0f);
        ImGui::SliderInt("Speed (ops/frame)", &dbg.instructionsPerFrame, 1, 10000);

        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 170.0f);
        ImGui::SetNextItemWidth(70.0f);
        ImGui::InputInt("##StepCount", &g_StepCount, 0);
        ImGui::SameLine();
        if (ImGui::Button("Step N")) {
            if (g_StepCount > 0) dbg.ExecuteInstructions(g_StepCount);
        }

        ImGui::SetNextItemWidth(180.0f);
        ImGui::InputInt("Cycle Budget", &g_CycleBudget, 0);
        ImGui::SameLine();
        if (ImGui::Button("Run Cycles")) {
            if (g_CycleBudget > 0) dbg.ExecuteCycles(g_CycleBudget);
        }
    } else {
        ImGui::PopStyleColor();
    }
}

void DebuggerUI::RenderDisassembly(Debugger& dbg) {
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.18f, 0.22f, 0.28f, 1.0f));
    if (ImGui::CollapsingHeader("DISASSEMBLY VIEWER", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::PopStyleColor();

        // Disassembly controls bar
        ImGui::Checkbox("Follow PC", &g_FollowPC);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(70.0f);
        ImGui::InputText("##DisasmJump", g_DisasmJumpBuffer, sizeof(g_DisasmJumpBuffer));
        ImGui::SameLine();
        if (ImGui::Button("Goto Addr")) {
            g_FollowPC = false;
        }

        Word centerAddr = g_FollowPC ? dbg.cpu.PC : ParseHex(g_DisasmJumpBuffer, dbg.cpu.PC);

        // Disassemble instructions around centerAddr
        auto instructions = Disassembler::DisassembleAround(dbg.memory, centerAddr, 10, 30);

        ImGuiTableFlags tableFlags = ImGuiTableFlags_RowBg | 
                                     ImGuiTableFlags_BordersInnerV | 
                                     ImGuiTableFlags_ScrollY |
                                     ImGuiTableFlags_SizingFixedFit;

        float tableHeight = ImGui::GetContentRegionAvail().y - 10.0f;
        if (ImGui::BeginTable("DisassemblyTable", 5, tableFlags, ImVec2(0, tableHeight))) {
            ImGui::TableSetupColumn("BP", ImGuiTableColumnFlags_WidthFixed, 25.0f);
            ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed, 65.0f);
            ImGui::TableSetupColumn("Bytes", ImGuiTableColumnFlags_WidthFixed, 90.0f);
            ImGui::TableSetupColumn("Instruction", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Mode", ImGuiTableColumnFlags_WidthFixed, 90.0f);
            ImGui::TableHeadersRow();

            for (const auto& inst : instructions) {
                ImGui::TableNextRow();

                bool isCurrentPC = (inst.address == dbg.cpu.PC);
                bool hasBP = dbg.HasBreakpoint(inst.address);

                // Row highlighting
                if (isCurrentPC) {
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImColor(30, 90, 160, 180)); // Cyan-Blue for current PC
                } else if (hasBP) {
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImColor(160, 40, 40, 100)); // Red for Breakpoint
                }

                // Column 0: Breakpoint Toggle
                ImGui::TableSetColumnIndex(0);
                ImGui::PushID(inst.address);
                if (hasBP) {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
                    if (ImGui::Selectable(" ● ", false, ImGuiSelectableFlags_SpanAllColumns)) {
                        dbg.ToggleBreakpoint(inst.address);
                    }
                    ImGui::PopStyleColor();
                } else {
                    if (ImGui::Selectable("   ", false, ImGuiSelectableFlags_SpanAllColumns)) {
                        dbg.ToggleBreakpoint(inst.address);
                    }
                }
                ImGui::PopID();

                // Column 1: Address
                ImGui::TableSetColumnIndex(1);
                if (isCurrentPC) {
                    ImGui::TextColored(ImVec4(0.4f, 0.9f, 1.0f, 1.0f), "=>%04X", inst.address);
                } else {
                    ImGui::Text("%04X", inst.address);
                }

                // Column 2: Bytes
                ImGui::TableSetColumnIndex(2);
                ImGui::TextDisabled("%s", inst.bytesHex.c_str());

                // Column 3: Mnemonic & Operand
                ImGui::TableSetColumnIndex(3);
                if (isCurrentPC) {
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", inst.fullText.c_str());
                } else {
                    ImGui::Text("%s", inst.fullText.c_str());
                }

                // Column 4: Addressing Mode
                ImGui::TableSetColumnIndex(4);
                ImGui::TextDisabled("%s", Disassembler::GetAddressingModeName(inst.mode));

                if (isCurrentPC && g_FollowPC) {
                    ImGui::SetScrollHereY(0.35f);
                }
            }

            ImGui::EndTable();
        }
    } else {
        ImGui::PopStyleColor();
    }
}

void DebuggerUI::RenderMemoryViewer(Debugger& dbg) {
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.18f, 0.22f, 0.28f, 1.0f));
    if (ImGui::CollapsingHeader("HEXADECIMAL MEMORY VIEWER (64KB)", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::PopStyleColor();

        // Memory navigation bar
        ImGui::Text("Goto Address:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(70.0f);
        if (ImGui::InputText("##MemJump", g_MemJumpBuffer, sizeof(g_MemJumpBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
            g_SelectedMemAddress = ParseHex(g_MemJumpBuffer, g_SelectedMemAddress);
        }
        ImGui::SameLine();
        if (ImGui::Button("Jump")) {
            g_SelectedMemAddress = ParseHex(g_MemJumpBuffer, g_SelectedMemAddress);
        }
        ImGui::SameLine();
        if (ImGui::Button("Jump to PC")) {
            g_SelectedMemAddress = dbg.cpu.PC;
            snprintf(g_MemJumpBuffer, sizeof(g_MemJumpBuffer), "%04X", dbg.cpu.PC);
        }
        ImGui::SameLine();
        if (ImGui::Button("Zero Page")) {
            g_SelectedMemAddress = 0x0000;
        }
        ImGui::SameLine();
        if (ImGui::Button("Stack ($0100)")) {
            g_SelectedMemAddress = 0x0100;
        }

        // 16 bytes per row, total 4096 rows
        ImGuiTableFlags flags = ImGuiTableFlags_RowBg | 
                                ImGuiTableFlags_BordersInnerV | 
                                ImGuiTableFlags_ScrollY | 
                                ImGuiTableFlags_SizingFixedFit;

        float tableHeight = 220.0f;
        if (ImGui::BeginTable("MemoryTable", 18, flags, ImVec2(0, tableHeight))) {
            ImGui::TableSetupColumn("Addr", ImGuiTableColumnFlags_WidthFixed, 45.0f);
            for (int col = 0; col < 16; ++col) {
                char colName[4];
                snprintf(colName, sizeof(colName), "%02X", col);
                ImGui::TableSetupColumn(colName, ImGuiTableColumnFlags_WidthFixed, 20.0f);
            }
            ImGui::TableSetupColumn("ASCII", ImGuiTableColumnFlags_WidthFixed, 130.0f);
            ImGui::TableHeadersRow();

            // Use ImGuiListClipper for high-performance virtual scrolling of 4096 rows
            ImGuiListClipper clipper;
            clipper.Begin(4096);

            while (clipper.Step()) {
                for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
                    Word rowBaseAddr = static_cast<Word>(row * 16);
                    ImGui::TableNextRow();

                    // Address column
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%04X", rowBaseAddr);

                    // 16 Hex Byte columns
                    char asciiBuf[17];
                    asciiBuf[16] = '\0';

                    for (int col = 0; col < 16; ++col) {
                        ImGui::TableSetColumnIndex(1 + col);
                        Word byteAddr = static_cast<Word>(rowBaseAddr + col);
                        Byte val = dbg.memory[byteAddr];

                        // ASCII representation
                        asciiBuf[col] = (val >= 32 && val <= 126) ? static_cast<char>(val) : '.';

                        bool isPC = (byteAddr == dbg.cpu.PC);
                        bool isSP = (byteAddr == (0x0100 | dbg.cpu.SP));
                        bool isSelected = (byteAddr == g_SelectedMemAddress);

                        if (isPC) {
                            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImColor(40, 120, 220, 180));
                        } else if (isSP) {
                            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImColor(40, 180, 80, 140));
                        } else if (isSelected) {
                            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImColor(180, 140, 30, 160));
                        }

                        char byteStr[4];
                        snprintf(byteStr, sizeof(byteStr), "%02X", val);

                        ImGui::PushID(byteAddr);
                        if (ImGui::Selectable(byteStr, isSelected, ImGuiSelectableFlags_None, ImVec2(18, 0))) {
                            g_SelectedMemAddress = byteAddr;
                            g_MemEditing = true;
                            g_MemEditAddr = byteAddr;
                            snprintf(g_MemEditValBuffer, sizeof(g_MemEditValBuffer), "%02X", val);
                        }
                        ImGui::PopID();
                    }

                    // ASCII column
                    ImGui::TableSetColumnIndex(17);
                    ImGui::TextDisabled("%s", asciiBuf);
                }
            }
            ImGui::EndTable();
        }

        // Inline memory editor bar
        if (g_MemEditing) {
            ImGui::Text("Edit Byte at 0x%04X: ", g_MemEditAddr);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(50.0f);
            if (ImGui::InputText("##EditVal", g_MemEditValBuffer, sizeof(g_MemEditValBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
                Byte newVal = static_cast<Byte>(ParseHex(g_MemEditValBuffer, dbg.memory[g_MemEditAddr]) & 0xFF);
                dbg.memory[g_MemEditAddr] = newVal;
                g_MemEditing = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Write")) {
                Byte newVal = static_cast<Byte>(ParseHex(g_MemEditValBuffer, dbg.memory[g_MemEditAddr]) & 0xFF);
                dbg.memory[g_MemEditAddr] = newVal;
                g_MemEditing = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                g_MemEditing = false;
            }
        }
    } else {
        ImGui::PopStyleColor();
    }
}

void DebuggerUI::RenderStackViewer(Debugger& dbg) {
    ImGui::TextColored(ImVec4(0.25f, 0.75f, 1.0f, 1.0f), "STACK VIEWER ($0100 - $01FF)");
    ImGui::SameLine();
    ImGui::TextDisabled("| SP: 0x%02X (0x%04X)", dbg.cpu.SP, 0x0100 | dbg.cpu.SP);

    ImGuiTableFlags flags = ImGuiTableFlags_RowBg | 
                            ImGuiTableFlags_BordersInnerV | 
                            ImGuiTableFlags_ScrollY |
                            ImGuiTableFlags_SizingFixedFit;

    if (ImGui::BeginTable("StackTable", 4, flags, ImVec2(0, 0))) {
        ImGui::TableSetupColumn("Pointer", ImGuiTableColumnFlags_WidthFixed, 65.0f);
        ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed, 65.0f);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, 55.0f);
        ImGui::TableSetupColumn("Decimal", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        // 6502 stack goes from 0x01FF down to 0x0100
        for (int i = 0xFF; i >= 0x00; --i) {
            Word addr = static_cast<Word>(0x0100 | i);
            Byte val = dbg.memory[addr];
            bool isCurrentSP = (i == dbg.cpu.SP);

            ImGui::TableNextRow();
            if (isCurrentSP) {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImColor(40, 160, 80, 120)); // Highlight current SP
            }

            ImGui::TableSetColumnIndex(0);
            if (isCurrentSP) {
                ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.5f, 1.0f), "==> SP");
            } else if (i > dbg.cpu.SP) {
                ImGui::TextDisabled(" pushed");
            } else {
                ImGui::TextDisabled(" empty");
            }

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%04X", addr);

            ImGui::TableSetColumnIndex(2);
            ImGui::Text("0x%02X", val);

            ImGui::TableSetColumnIndex(3);
            ImGui::TextDisabled("(%d)", static_cast<int>(val));
        }

        ImGui::EndTable();
    }
}

void DebuggerUI::RenderBreakpointsAndStats(Debugger& dbg) {
    ImGui::TextColored(ImVec4(0.25f, 0.75f, 1.0f, 1.0f), "BREAKPOINTS & EXECUTION STATS");

    // Two columns inside stats: left stats, right breakpoints
    ImGui::Columns(2, "StatsCol", false);

    // Left Column: Execution Statistics
    ImGui::Text("Execution Stats:");
    ImGui::Text("  Status:       %s", (dbg.state == DebuggerState::Running) ? "RUNNING" : "PAUSED");
    ImGui::Text("  Instructions: %llu", static_cast<unsigned long long>(dbg.totalInstructions));
    ImGui::Text("  Clock Cycles: %llu", static_cast<unsigned long long>(dbg.totalCycles));
    ImGui::Text("  Last Opcode:  0x%02X", dbg.lastOpcode);
    ImGui::Text("  Last PC:      0x%04X", dbg.lastPC);

    ImGui::NextColumn();

    // Right Column: Active Breakpoints
    ImGui::Text("Active Breakpoints (%zu):", dbg.breakpoints.size());
    ImGui::SetNextItemWidth(70.0f);
    ImGui::InputText("##AddBP", g_BreakpointInputBuffer, sizeof(g_BreakpointInputBuffer));
    ImGui::SameLine();
    if (ImGui::Button("Add BP")) {
        Word bpAddr = ParseHex(g_BreakpointInputBuffer, 0);
        dbg.AddBreakpoint(bpAddr);
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear All")) {
        dbg.ClearBreakpoints();
    }

    if (ImGui::BeginChild("BPList", ImVec2(0, 80), true)) {
        std::vector<Word> toRemove;
        for (Word bp : dbg.breakpoints) {
            char bpLabel[32];
            snprintf(bpLabel, sizeof(bpLabel), "0x%04X", bp);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
            ImGui::Text("● %s", bpLabel);
            ImGui::PopStyleColor();
            ImGui::SameLine(ImGui::GetContentRegionAvail().x - 45.0f);
            char btnId[16];
            snprintf(btnId, sizeof(btnId), "X##%04X", bp);
            if (ImGui::SmallButton(btnId)) {
                toRemove.push_back(bp);
            }
        }
        for (Word bp : toRemove) {
            dbg.RemoveBreakpoint(bp);
        }
    }
    ImGui::EndChild();

    ImGui::Columns(1);
}
