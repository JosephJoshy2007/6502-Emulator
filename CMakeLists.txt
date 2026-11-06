cmake_minimum_required(VERSION 3.14)
project(6502Emulator CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(POLICY CMP0169)
  cmake_policy(SET CMP0169 OLD)
endif()

# 1. Build the emulator as a static library
add_library(6502_lib m6502.cpp)
target_include_directories(6502_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# 2. Build the main executable (for the actual app later)
add_executable(6502_app main.cpp)
target_link_libraries(6502_app PRIVATE 6502_lib)

# 3. Automatically fetch Google Test from GitHub
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

add_executable(6502_load_tests testing/test_load.cpp)
target_link_libraries(6502_load_tests PRIVATE 6502_lib GTest::gtest_main)

add_executable(6502_store_tests testing/test_store.cpp)
target_link_libraries(6502_store_tests PRIVATE 6502_lib GTest::gtest_main)

add_executable(6502_jump_tests testing/test_jump.cpp)
target_link_libraries(6502_jump_tests PRIVATE 6502_lib GTest::gtest_main)

add_executable(6502_stack_tests testing/test_stack.cpp)
target_link_libraries(6502_stack_tests PRIVATE 6502_lib GTest::gtest_main)

add_executable(6502_logical_tests testing/test_logical.cpp)
target_link_libraries(6502_logical_tests PRIVATE 6502_lib GTest::gtest_main)

add_executable(6502_register_tests testing/test_register.cpp)
target_link_libraries(6502_register_tests PRIVATE 6502_lib GTest::gtest_main)

add_executable(6502_incdec_tests testing/test_incdec.cpp)
target_link_libraries(6502_incdec_tests PRIVATE 6502_lib GTest::gtest_main)

add_executable(6502_branch_tests testing/test_branch.cpp)
target_link_libraries(6502_branch_tests PRIVATE 6502_lib GTest::gtest_main)

add_executable(6502_set_clear_tests testing/test_set_clear.cpp)
target_link_libraries(6502_set_clear_tests PRIVATE 6502_lib GTest::gtest_main)

add_executable(6502_addcarry_tests testing/test_addcarry.cpp)
target_link_libraries(6502_addcarry_tests PRIVATE 6502_lib GTest::gtest_main)

add_executable(6502_compare_tests testing/test_compare.cpp)
target_link_libraries(6502_compare_tests PRIVATE 6502_lib GTest::gtest_main)

add_executable(6502_subtractcarry_tests testing/test_subtractcarry.cpp)
target_link_libraries(6502_subtractcarry_tests PRIVATE 6502_lib GTest::gtest_main)

add_executable(6502_shift_left_tests testing/test_shift_left.cpp)
target_link_libraries(6502_shift_left_tests PRIVATE 6502_lib GTest::gtest_main)

add_executable(6502_shift_right_tests testing/test_shift_right.cpp)
target_link_libraries(6502_shift_right_tests PRIVATE 6502_lib GTest::gtest_main)

add_executable(6502_rotate_tests testing/test_rotate.cpp)
target_link_libraries(6502_rotate_tests PRIVATE 6502_lib GTest::gtest_main)

add_executable(6502_interrupt_tests testing/test_interrupt.cpp)
target_link_libraries(6502_interrupt_tests PRIVATE 6502_lib GTest::gtest_main)

# 4. GUI Debugger (GLFW + OpenGL + Dear ImGui)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.91.5
)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
  FetchContent_Populate(imgui)
  add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
  )
  target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
  )
  target_link_libraries(imgui PUBLIC glfw)
  if(WIN32)
    target_link_libraries(imgui PUBLIC opengl32)
  endif()
endif()

add_executable(6502_debugger
  debugger/Disassembler.cpp
  debugger/Debugger.cpp
  debugger/DebuggerUI.cpp
  debugger/main_gui.cpp
)
target_include_directories(6502_debugger PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/debugger
)
target_link_libraries(6502_debugger PRIVATE
  6502_lib
  imgui
  glfw
)
if(WIN32)
  target_link_libraries(6502_debugger PRIVATE opengl32)
endif()
