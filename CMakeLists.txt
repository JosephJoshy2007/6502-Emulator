cmake_minimum_required(VERSION 3.14)
project(6502Emulator CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Build the emulator as a static library
add_library(6502_lib m6502.cpp)

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

# 4. Build the test executable
add_executable(6502_tests test.cpp)
# Linking GTest::gtest_main automatically provides the main() function for the tests
target_link_libraries(6502_tests PRIVATE 6502_lib GTest::gtest_main)