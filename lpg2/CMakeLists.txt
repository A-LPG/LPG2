cmake_minimum_required(VERSION 3.16)

set(LPG2_VERSION "2.3.0")
project(lpg2 VERSION ${LPG2_VERSION} LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(LPG2_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(LPG2_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
configure_file(
    "${LPG2_INCLUDE_DIR}/lpg_version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/generated/lpg_version.h"
    @ONLY)
add_executable(lpg2 "")

### Includes
target_include_directories(lpg2 PUBLIC ${LPG2_INCLUDE_DIR})
target_include_directories(lpg2 PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}/generated")
# Sources for the executable are specified at end of CMakeLists.txt
if(MSVC)
target_compile_options(lpg2 PRIVATE /W4)
else()
target_compile_options(lpg2 PRIVATE "-Wall" "-Wextra")
endif()

# Source-tree template root for out-of-tree builds (CI uses -B $RUNNER_TEMP/...).
set(LPG2_TEMPLATE_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/../lpg-generator-templates-2.1.00")
# Prefer forward slashes so the generated header is valid on Windows MSVC.
file(TO_CMAKE_PATH "${LPG2_TEMPLATE_ROOT}" LPG2_TEMPLATE_ROOT_NATIVE)
configure_file(
    "${LPG2_INCLUDE_DIR}/lpg_resource_root.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/generated/lpg_resource_root.h"
    @ONLY)
if(MSVC)
    target_compile_definitions(lpg2 PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

option(LPG2_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
if(LPG2_WARNINGS_AS_ERRORS)
    if(MSVC)
        target_compile_options(lpg2 PRIVATE /WX)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # Ubuntu CI (GCC): promote more classes to errors after 2.3.0 cleanup.
        # null-dereference stays a warning only: GCC false-positives on
        # BlockSymbol::BlockBegin() inlines across *Action.cpp.
        target_compile_options(lpg2 PRIVATE
            -Werror=return-type
            -Werror=uninitialized
            -Werror=sequence-point
            -Werror=format
            -Werror=unused-variable
            -Werror=unused-but-set-variable
            -Wno-deprecated-declarations)
    else()
        # Clang/AppleClang: match GCC's promoted set where supported.
        target_compile_options(lpg2 PRIVATE
            -Werror=return-type
            -Werror=uninitialized
            -Werror=format
            -Wno-deprecated-declarations)
    endif()
endif()

option(LPG2_ENABLE_SANITIZERS
    "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
if(LPG2_ENABLE_SANITIZERS)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(lpg2 PRIVATE
            -fsanitize=address,undefined
            -fno-omit-frame-pointer)
        target_link_options(lpg2 PRIVATE
            -fsanitize=address,undefined
            -fno-omit-frame-pointer)
    else()
        message(FATAL_ERROR
            "LPG2_ENABLE_SANITIZERS requires Clang or GCC")
    endif()
endif()

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
        src/RustAction.cpp
        src/RustTable.cpp
        src/grammar.cpp
        src/JavaAction.cpp
        src/JavaTable.cpp
        src/jikespg.cpp
        src/jikespg_act.cpp
        src/jikespg_init.cpp
        src/jikespg_prs.cpp
        src/LexStream.cpp
        src/option.cpp
        src/optionDesc.cpp
        src/optionParser.cpp
        src/options.cpp
        src/optionValues.cpp
        src/output_transaction.cpp
        src/parser.cpp
        src/pda.cpp
        src/produce.cpp
        src/Python2Action.cpp
        src/Python2Table.cpp
        src/Python3Action.cpp
        src/Python3Table.cpp
        src/resolve.cpp
        src/resource_paths.cpp
        src/scanner.cpp
        src/sp.cpp
        src/symbol.cpp
        src/tab.cpp
        src/table.cpp
        src/TypeScriptAction.cpp
        src/TypeScriptTable.cpp
        src/util.cpp
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

set_target_properties(lpg2 PROPERTIES OUTPUT_NAME "lpg-v${LPG2_VERSION}")

set(LPG2_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}"
    CACHE PATH "lpg2 binary output directory")
set(LPG2_DEPLOY_DIR ""
    CACHE PATH "Optional directory receiving the built lpg2 executable")
if(LPG2_DEPLOY_DIR)
    set_output_directory(lpg2 BINARY_DIR "${LPG2_DEPLOY_DIR}")
else()
    set_output_directory(lpg2 BINARY_DIR "${LPG2_OUTPUT_DIR}")
endif()

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

include(GNUInstallDirs)
install(TARGETS lpg2
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")

set(LPG2_TEMPLATE_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/../lpg-generator-templates-2.1.00")
install(DIRECTORY
    "${LPG2_TEMPLATE_ROOT}/templates"
    "${LPG2_TEMPLATE_ROOT}/include"
    DESTINATION
        "${CMAKE_INSTALL_DATADIR}/lpg2/lpg-generator-templates-2.1.00")
install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/../README.md"
    "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE"
    "${CMAKE_CURRENT_SOURCE_DIR}/../docs/USER.md"
    DESTINATION "${CMAKE_INSTALL_DOCDIR}")

set(CPACK_PACKAGE_NAME "lpg2")
set(CPACK_PACKAGE_VENDOR "LPG Group")
set(CPACK_PACKAGE_VERSION "${LPG2_VERSION}")
set(CPACK_PACKAGE_FILE_NAME
    "lpg2-${LPG2_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_PACKAGE_CHECKSUM "SHA256")
if(WIN32)
    set(CPACK_GENERATOR "ZIP")
else()
    set(CPACK_GENERATOR "TGZ")
endif()
include(CPack)