cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 17)

project(lpg2 C CXX)

set(LPG2_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(LPG2_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_executable(lpg2 "")

# CppUTest
include(FetchContent)
FetchContent_Declare(
        inja
        GIT_REPOSITORY https://gitee.com/snooper-project/inja.git
)
FetchContent_MakeAvailable(inja)

### Includes
target_include_directories(lpg2 PUBLIC ${LPG2_INCLUDE_DIR})
# Sources for the executable are specified at end of CMakeLists.txt
if(MSVC)
target_compile_options(lpg2 PRIVATE "-W4")
else()
target_compile_options(lpg2 PRIVATE "-Wall" "-Wextra")
endif()

target_include_directories(lpg2 PRIVATE jnja)


if(CMAKE_BUILD_TYPE MATCHES Debug)
target_compile_definitions(lpg2 PRIVATE _DEBUG)
endif()

target_sources(lpg2 PRIVATE
        src/Action.cpp
        src/base.cpp
        src/buffer.cpp
        src/CAction.cpp
        src/code.cpp
        src/control.cpp
        src/CppAction.cpp
        src/CppAction2.cpp
        src/CppTable.cpp
        src/CppTable2.cpp
        src/CSharpAction.cpp
        src/CSharpTable.cpp
        src/CTable.cpp
        src/DartAction.cpp
        src/DartTable.cpp
        src/dfa.cpp
        src/diagnose.cpp
        src/dump.cpp
        src/generator.cpp
        src/GoAction.cpp
        src/GoTable.cpp
        src/grammar.cpp
        src/JavaAction.cpp
        src/JavaTable.cpp
        src/jikespg.cpp
        src/jikespg_act.cpp
        src/jikespg_init.cpp
        src/jikespg_prs.cpp
        src/LexStream.cpp
        src/MlAction.cpp
        src/MlTable.cpp
        src/option.cpp
        src/optionDesc.cpp
        src/optionParser.cpp
        src/options.cpp
        src/optionValues.cpp
        src/parser.cpp
        src/pda.cpp
        src/PlxAction.cpp
        src/PlxasmAction.cpp
        src/PlxasmTable.cpp
        src/PlxTable.cpp
        src/produce.cpp
        src/Python2Action.cpp
        src/Python2Table.cpp
        src/Python3Action.cpp
        src/Python3Table.cpp
        src/resolve.cpp
        src/scanner.cpp
        src/sp.cpp
        src/symbol.cpp
        src/tab.cpp
        src/table.cpp
        src/TypeScriptAction.cpp
        src/TypeScriptTable.cpp
        src/util.cpp
        src/XmlAction.cpp
        src/XmlTable.cpp
        )

