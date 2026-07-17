cmake_minimum_required(VERSION 3.16)
project(calculator_cpp_example LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

if(NOT DEFINED LPG2_CPP_RUNTIME_DIR)
  set(LPG2_CPP_RUNTIME_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../runtime/LPG-cpp-runtime")
endif()
if(NOT EXISTS "${LPG2_CPP_RUNTIME_DIR}/CMakeLists.txt")
  message(FATAL_ERROR "LPG-cpp-runtime not found at ${LPG2_CPP_RUNTIME_DIR}")
endif()

add_subdirectory("${LPG2_CPP_RUNTIME_DIR}" cpplpg2_build EXCLUDE_FROM_ALL)

set(GEN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../out-cpp")
add_executable(calculator_check
  main.cpp
  "${GEN_DIR}/calculator.cpp"
)
target_include_directories(calculator_check PRIVATE "${GEN_DIR}")
target_link_libraries(calculator_check PRIVATE cpplpg2)
