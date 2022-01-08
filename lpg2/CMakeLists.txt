cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 17)

project(lpg2 C CXX)

set(LPG2_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(LPG2_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
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
        src/VisitorStaffFactory.cpp
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


# Set each output directory according to ${CMAKE_CONFIGURATION_TYPES}.
# Note: Don't set variables CMAKE_*_OUTPUT_DIRECTORY any more,
# or a certain builder, for eaxample, msbuild.exe, would be confused.
function(set_output_directory target)
    cmake_parse_arguments(ARG "" "BINARY_DIR;LIBRARY_DIR" "" ${ARGN})

    # module_dir -- corresponding to LIBRARY_OUTPUT_DIRECTORY.
    # It affects output of add_library(MODULE).
    if(WIN32 OR CYGWIN)
        # DLL platform
        set(module_dir ${ARG_BINARY_DIR})
    else()
        set(module_dir ${ARG_LIBRARY_DIR})
    endif()
    if(NOT "${CMAKE_CFG_INTDIR}" STREQUAL ".")
        foreach(build_mode ${CMAKE_CONFIGURATION_TYPES})
            string(TOUPPER "${build_mode}" CONFIG_SUFFIX)
            if(ARG_BINARY_DIR)
                string(REPLACE ${CMAKE_CFG_INTDIR} ${build_mode} bi ${ARG_BINARY_DIR})
                set_target_properties(${target} PROPERTIES "RUNTIME_OUTPUT_DIRECTORY_${CONFIG_SUFFIX}" ${bi})
            endif()
            if(ARG_LIBRARY_DIR)
                string(REPLACE ${CMAKE_CFG_INTDIR} ${build_mode} li ${ARG_LIBRARY_DIR})
                set_target_properties(${target} PROPERTIES "ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_SUFFIX}" ${li})
            endif()
            if(module_dir)
                string(REPLACE ${CMAKE_CFG_INTDIR} ${build_mode} mi ${module_dir})
                set_target_properties(${target} PROPERTIES "LIBRARY_OUTPUT_DIRECTORY_${CONFIG_SUFFIX}" ${mi})
            endif()
        endforeach()
    else()
        if(ARG_BINARY_DIR)
            set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${ARG_BINARY_DIR})
        endif()
        if(ARG_LIBRARY_DIR)
            set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${ARG_LIBRARY_DIR})
        endif()
        if(module_dir)
            set_target_properties(${target} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${module_dir})
        endif()
    endif()
endfunction()

set_target_properties(lpg2 PROPERTIES OUTPUT_NAME "lpg-v2.1.09")
set_output_directory(lpg2 BINARY_DIR "/Users/kuafu/.vscode/extensions/kuafuwang.lpg-vscode-0.0.16/server/mac")