set(LPG2_TEST_RUNNER "${CMAKE_CURRENT_SOURCE_DIR}/run_generation_test.cmake")
set(LPG2_FIXTURES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/fixtures")
set(LPG2_CTEST_OUT_ROOT "${CMAKE_BINARY_DIR}/ctest")

add_executable(lpg2_container_tests container_tests.cpp)
target_include_directories(lpg2_container_tests PRIVATE
    "${CMAKE_SOURCE_DIR}/include")
target_compile_features(lpg2_container_tests PRIVATE cxx_std_17)
if(MSVC)
    target_compile_options(lpg2_container_tests PRIVATE /W4)
    target_compile_definitions(lpg2_container_tests PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(lpg2_container_tests PRIVATE
        -Wall -Wextra -Werror)
endif()
if(LPG2_ENABLE_SANITIZERS)
    target_compile_options(lpg2_container_tests PRIVATE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer)
    target_link_options(lpg2_container_tests PRIVATE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer)
endif()
add_test(NAME core_containers COMMAND lpg2_container_tests)
set_tests_properties(core_containers PROPERTIES
    LABELS "containers;unit")

add_executable(lpg2_output_transaction_tests
    output_transaction_tests.cpp
    "${CMAKE_SOURCE_DIR}/src/output_transaction.cpp")
target_include_directories(lpg2_output_transaction_tests PRIVATE
    "${CMAKE_SOURCE_DIR}/include")
target_compile_features(lpg2_output_transaction_tests PRIVATE cxx_std_17)
if(MSVC)
    target_compile_options(lpg2_output_transaction_tests PRIVATE /W4)
    target_compile_definitions(lpg2_output_transaction_tests PRIVATE
        _CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(lpg2_output_transaction_tests PRIVATE
        -Wall -Wextra -Werror)
endif()
if(LPG2_ENABLE_SANITIZERS)
    target_compile_options(lpg2_output_transaction_tests PRIVATE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer)
    target_link_options(lpg2_output_transaction_tests PRIVATE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer)
endif()
add_test(NAME output_transaction COMMAND lpg2_output_transaction_tests)
set_tests_properties(output_transaction PROPERTIES
    LABELS "output;unit")

add_executable(lpg2_resource_path_tests
    resource_path_tests.cpp
    "${CMAKE_SOURCE_DIR}/src/resource_paths.cpp")
target_include_directories(lpg2_resource_path_tests PRIVATE
    "${CMAKE_SOURCE_DIR}/include")
target_compile_features(lpg2_resource_path_tests PRIVATE cxx_std_17)
# Bake the same source-tree template root used by the lpg2 binary so out-of-tree
# CI builds (cmake -B $RUNNER_TEMP/...) still discover templates.
target_include_directories(lpg2_resource_path_tests PRIVATE
    "${CMAKE_BINARY_DIR}/generated")
if(MSVC)
    target_compile_options(lpg2_resource_path_tests PRIVATE /W4)
    target_compile_definitions(lpg2_resource_path_tests PRIVATE
        _CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(lpg2_resource_path_tests PRIVATE
        -Wall -Wextra -Werror)
endif()
add_test(NAME resource_discovery COMMAND lpg2_resource_path_tests)
set_tests_properties(resource_discovery PROPERTIES
    LABELS "install;unit")

# Rust integration is enabled automatically when cargo and the runtime
# submodule under runtime/LPG-rust-runtime are available.
find_program(CARGO_EXECUTABLE cargo)
set(LPG2_RUST_TEST_RUNTIME_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/rust-runtime-shim"
    CACHE PATH "Dependency-free Rust runtime ABI shim used by table tests")
set(LPG2_RUST_RUNTIME_DIR
    "${CMAKE_SOURCE_DIR}/../runtime/LPG-rust-runtime/lpg2"
    CACHE PATH "Path to the LPG Rust runtime crate")
option(LPG2_REQUIRE_RUST_TESTS
    "Fail configure when Rust table compile/run tests cannot be enabled" OFF)
option(LPG2_REQUIRE_RUST_PARSER_TESTS
    "Fail configure when full Rust runtime parser tests cannot be enabled" OFF)

# C++ runtime parser/AST tests link against LPG-cpp-runtime (cpplpg2).
# CMAKE_SOURCE_DIR is lpg2/, so ../runtime is LPG2/runtime.
set(LPG2_CPP_RUNTIME_DIR
    "${CMAKE_SOURCE_DIR}/../runtime/LPG-cpp-runtime"
    CACHE PATH "Path to LPG C++ runtime (cpplpg2)")
option(LPG2_REQUIRE_CPP_PARSER_TESTS
    "Fail configure when C++ runtime parser/AST tests cannot be enabled" OFF)

# C++ runtime token-stream incremental reset (requires LPG-cpp-runtime).
if(EXISTS "${LPG2_CPP_RUNTIME_DIR}/CMakeLists.txt")
    add_executable(lpg2_incremental_prs_stream_tests
        incremental_prs_stream_tests.cpp)
    set(CPPLPG2_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(CPPLPG2_INSTALL OFF CACHE BOOL "" FORCE)
    if(NOT TARGET cpplpg2)
        add_subdirectory("${LPG2_CPP_RUNTIME_DIR}"
            "${CMAKE_CURRENT_BINARY_DIR}/cpplpg2_incremental"
            EXCLUDE_FROM_ALL)
    endif()
    target_link_libraries(lpg2_incremental_prs_stream_tests PRIVATE cpplpg2)
    target_compile_features(lpg2_incremental_prs_stream_tests PRIVATE cxx_std_17)
    # Runtime headers predate -Werror=inconsistent-missing-override; match
    # generated-parser harness (silence runtime noise, keep test sources clean).
    if(MSVC)
        target_compile_options(lpg2_incremental_prs_stream_tests PRIVATE /W3)
        target_compile_definitions(lpg2_incremental_prs_stream_tests PRIVATE
            _CRT_SECURE_NO_WARNINGS)
    else()
        # Match generated-parser harness: runtime headers are noisy.
        target_compile_options(lpg2_incremental_prs_stream_tests PRIVATE -w)
    endif()
    add_test(NAME incremental_prs_stream
        COMMAND lpg2_incremental_prs_stream_tests)
    set_tests_properties(incremental_prs_stream PROPERTIES
        LABELS "incremental;unit;cpp")
endif()

# Java runtime parser/AST tests compile against runtime/lpg-runtime (javac/java).
set(LPG2_JAVA_RUNTIME_DIR
    "${CMAKE_SOURCE_DIR}/../runtime/lpg-runtime"
    CACHE PATH "Path to LPG Java runtime (lpg-runtime)")
option(LPG2_REQUIRE_JAVA_PARSER_TESTS
    "Fail configure when Java runtime parser/AST tests cannot be enabled" OFF)
find_program(Java_JAVAC_EXECUTABLE javac)
find_program(Java_JAVA_EXECUTABLE java)

# Python3 runtime parser/AST tests (PYTHONPATH = runtime/LPG-python-runtime/Python3/src).
set(LPG2_PYTHON_RUNTIME_DIR
    "${CMAKE_SOURCE_DIR}/../runtime/LPG-python-runtime/Python3/src"
    CACHE PATH "Path to LPG Python3 runtime package root (contains lpg2/)")
option(LPG2_REQUIRE_PYTHON_PARSER_TESTS
    "Fail configure when Python runtime parser/AST tests cannot be enabled" OFF)
find_program(Python3_EXECUTABLE NAMES python3 python)

# Go runtime parser/AST tests (module replace → runtime/LPG-go-runtime).
set(LPG2_GO_RUNTIME_DIR
    "${CMAKE_SOURCE_DIR}/../runtime/LPG-go-runtime"
    CACHE PATH "Path to LPG Go runtime module root (contains go.mod + lpg2/)")
option(LPG2_REQUIRE_GO_PARSER_TESTS
    "Fail configure when Go runtime parser/AST tests cannot be enabled" OFF)
find_program(GO_EXECUTABLE go)

# TypeScript runtime parser/AST tests (npm file: → runtime/LPG-typescript-runtime/lpg2ts).
set(LPG2_TYPESCRIPT_RUNTIME_DIR
    "${CMAKE_SOURCE_DIR}/../runtime/LPG-typescript-runtime/lpg2ts"
    CACHE PATH "Path to LPG TypeScript runtime package (lpg2ts)")
option(LPG2_REQUIRE_TYPESCRIPT_PARSER_TESTS
    "Fail configure when TypeScript runtime parser/AST tests cannot be enabled" OFF)
find_program(NODE_EXECUTABLE NAMES node)
find_program(NPM_EXECUTABLE NAMES npm)

# C# runtime parser/AST tests (ProjectReference → runtime/LPG-csharp-runtime/LPG2.Runtime).
set(LPG2_CSHARP_RUNTIME_DIR
    "${CMAKE_SOURCE_DIR}/../runtime/LPG-csharp-runtime/LPG2.Runtime"
    CACHE PATH "Path to LPG C# runtime project (LPG2.Runtime)")
option(LPG2_REQUIRE_CSHARP_PARSER_TESTS
    "Fail configure when C# runtime parser/AST tests cannot be enabled" OFF)
find_program(DOTNET_EXECUTABLE NAMES dotnet)

# Dart runtime parser/AST tests (path: → runtime/LPG-Dart-runtime).
set(LPG2_DART_RUNTIME_DIR
    "${CMAKE_SOURCE_DIR}/../runtime/LPG-Dart-runtime"
    CACHE PATH "Path to LPG Dart runtime package root (contains pubspec.yaml + lib/)")
option(LPG2_REQUIRE_DART_PARSER_TESTS
    "Fail configure when Dart runtime parser/AST tests cannot be enabled" OFF)
find_program(DART_EXECUTABLE NAMES dart)

# lpg2_add_generation_test(<name> <grammar> <lang>
#   [LABELS ...] [EXPECT_FAIL] [EXPECT_NO_OUTPUT] [CHECK_CPP] [CHECK_RUST]
#   [CHECK_JAVA] [CHECK_PYTHON] [CHECK_GO] [CHECK_TYPESCRIPT] [CHECK_CSHARP]
#   [CHECK_DART] [CHECK_GOLDEN]
#   [CPP_PARSE] [CPP_PARSER] [RUST_PARSE] [RUST_PARSER] [JAVA_PARSER]
#   [PYTHON_PARSER] [GO_PARSER] [TYPESCRIPT_PARSER] [CSHARP_PARSER] [DART_PARSER]
#   [PREFIX <prefix>] [EXPECT_EXIT_CODE <code>] [EXPECT_MESSAGE <substring>]
#   [TEMPLATE <path>] [INCLUDE_DIR <path>] [RUNTIME_DIR <crate/runtime path>]
#   [GOLDEN_DIR <dir>] [EXTRA_ARGS <cli args...>])
function(lpg2_add_generation_test NAME GRAMMAR LANG)
    set(options
        EXPECT_FAIL EXPECT_NO_OUTPUT CHECK_CPP CHECK_RUST CHECK_JAVA CHECK_PYTHON
        CHECK_GO CHECK_TYPESCRIPT CHECK_CSHARP CHECK_DART CHECK_GOLDEN
        CHECK_GLR_NEXTAST
        CPP_PARSE CPP_PARSER RUST_PARSE RUST_PARSER JAVA_PARSER PYTHON_PARSER
        GO_PARSER TYPESCRIPT_PARSER CSHARP_PARSER DART_PARSER)
    set(oneValueArgs
        PREFIX EXPECT_EXIT_CODE EXPECT_MESSAGE TEMPLATE INCLUDE_DIR
        RUNTIME_DIR GOLDEN_DIR RUST_PARSER_HARNESS JAVA_PARSER_HARNESS
        CPP_PARSER_HARNESS GO_PARSER_HARNESS TYPESCRIPT_PARSER_HARNESS
        CSHARP_PARSER_HARNESS DART_PARSER_HARNESS PYTHON_PARSER_HARNESS)
    set(multiValueArgs LABELS EXTRA_ARGS)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(_out_dir "${LPG2_CTEST_OUT_ROOT}/${NAME}")
    # All -D definitions must appear before -P (cmake script mode).
    set(_cmd
        ${CMAKE_COMMAND}
        "-DLPG2_EXE=$<TARGET_FILE:lpg2>"
        "-DGRAMMAR=${GRAMMAR}"
        "-DLANG=${LANG}"
        "-DOUT_DIR=${_out_dir}"
    )
    if(ARG_PREFIX)
        list(APPEND _cmd "-DEXPECT_PREFIX=${ARG_PREFIX}")
    endif()
    if(ARG_EXPECT_FAIL)
        list(APPEND _cmd "-DEXPECT_FAIL=TRUE")
    endif()
    if(ARG_EXPECT_NO_OUTPUT)
        list(APPEND _cmd "-DEXPECT_NO_OUTPUT=TRUE")
    endif()
    if(NOT "${ARG_EXPECT_EXIT_CODE}" STREQUAL "")
        list(APPEND _cmd "-DEXPECT_EXIT_CODE=${ARG_EXPECT_EXIT_CODE}")
    endif()
    if(NOT "${ARG_EXPECT_MESSAGE}" STREQUAL "")
        list(APPEND _cmd "-DEXPECT_MESSAGE=${ARG_EXPECT_MESSAGE}")
    endif()
    if(NOT "${ARG_TEMPLATE}" STREQUAL "")
        list(APPEND _cmd "-DTEMPLATE=${ARG_TEMPLATE}")
    endif()
    if(NOT "${ARG_INCLUDE_DIR}" STREQUAL "")
        list(APPEND _cmd "-DINCLUDE_DIR=${ARG_INCLUDE_DIR}")
    endif()
    if(ARG_EXTRA_ARGS)
        string(REPLACE ";" " " _extra_args_str "${ARG_EXTRA_ARGS}")
        list(APPEND _cmd "-DEXTRA_ARGS=${_extra_args_str}")
    endif()
    if(ARG_CHECK_GLR_NEXTAST)
        list(APPEND _cmd "-DCHECK_GLR_NEXTAST=TRUE")
    endif()
    if(ARG_CHECK_GOLDEN)
        if("${ARG_GOLDEN_DIR}" STREQUAL "")
            message(FATAL_ERROR
                "lpg2_add_generation_test(${NAME}): CHECK_GOLDEN requires GOLDEN_DIR")
        endif()
        list(APPEND _cmd
            "-DCHECK_GOLDEN=TRUE"
            "-DGOLDEN_DIR=${ARG_GOLDEN_DIR}")
    endif()
    if(ARG_CHECK_CPP)
        list(APPEND _cmd
            "-DCHECK_CPP=TRUE"
            "-DGENERATOR=${CMAKE_GENERATOR}"
            "-DGENERATOR_PLATFORM=${CMAKE_GENERATOR_PLATFORM}"
            "-DGENERATOR_TOOLSET=${CMAKE_GENERATOR_TOOLSET}"
            "-DTEST_CONFIG=$<CONFIG>"
            "-DCMAKE_CTEST_COMMAND=${CMAKE_CTEST_COMMAND}"
            "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}"
            "-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}")
        if(CMAKE_MAKE_PROGRAM)
            list(APPEND _cmd "-DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}")
        endif()
        if(ARG_CPP_PARSE)
            list(APPEND _cmd
                "-DCPP_PARSE=TRUE"
                "-DCPP_HARNESS=${CMAKE_CURRENT_SOURCE_DIR}/harness/cpp_table_harness.cpp.in")
        endif()
        if(ARG_CPP_PARSER)
            set(_cpp_runtime_dir "${LPG2_CPP_RUNTIME_DIR}")
            if(NOT "${ARG_RUNTIME_DIR}" STREQUAL "")
                set(_cpp_runtime_dir "${ARG_RUNTIME_DIR}")
            endif()
            set(_cpp_parser_harness
                "${CMAKE_CURRENT_SOURCE_DIR}/harness/cpp_parser_harness.cpp.in")
            if(NOT "${ARG_CPP_PARSER_HARNESS}" STREQUAL "")
                set(_cpp_parser_harness "${ARG_CPP_PARSER_HARNESS}")
            endif()
            list(APPEND _cmd
                "-DCPP_PARSER=TRUE"
                "-DCPP_HARNESS=${_cpp_parser_harness}"
                "-DCPP_RUNTIME_DIR=${_cpp_runtime_dir}")
        endif()
    endif()
    if(ARG_CHECK_RUST)
        set(_rust_runtime_dir "${LPG2_RUST_RUNTIME_DIR}")
        if(NOT "${ARG_RUNTIME_DIR}" STREQUAL "")
            set(_rust_runtime_dir "${ARG_RUNTIME_DIR}")
        endif()
        list(APPEND _cmd
            "-DCHECK_RUST=TRUE"
            "-DCARGO_EXECUTABLE=${CARGO_EXECUTABLE}"
            "-DRUST_RUNTIME_DIR=${_rust_runtime_dir}"
            "-DLPG2_CARGO_TARGET_DIR=${CMAKE_BINARY_DIR}/cargo-shared-target")
        if(ARG_RUST_PARSE)
            list(APPEND _cmd
                "-DRUST_PARSE=TRUE"
                "-DRUST_HARNESS=${CMAKE_CURRENT_SOURCE_DIR}/harness/rust_table_harness.rs.in")
        endif()
        if(ARG_RUST_PARSER)
            set(_rust_parser_harness
                "${CMAKE_CURRENT_SOURCE_DIR}/harness/rust_parser_harness.rs.in")
            if(NOT "${ARG_RUST_PARSER_HARNESS}" STREQUAL "")
                set(_rust_parser_harness "${ARG_RUST_PARSER_HARNESS}")
            endif()
            list(APPEND _cmd
                "-DRUST_PARSER=TRUE"
                "-DRUST_PARSER_HARNESS=${_rust_parser_harness}")
        endif()
    endif()
    if(ARG_CHECK_JAVA)
        set(_java_runtime_dir "${LPG2_JAVA_RUNTIME_DIR}")
        if(NOT "${ARG_RUNTIME_DIR}" STREQUAL "")
            set(_java_runtime_dir "${ARG_RUNTIME_DIR}")
        endif()
        list(APPEND _cmd
            "-DCHECK_JAVA=TRUE"
            "-DJAVA_JAVAC=${Java_JAVAC_EXECUTABLE}"
            "-DJAVA_JAVA=${Java_JAVA_EXECUTABLE}"
            "-DJAVA_RUNTIME_DIR=${_java_runtime_dir}")
        if(ARG_JAVA_PARSER)
            set(_java_parser_harness
                "${CMAKE_CURRENT_SOURCE_DIR}/harness/java_parser_harness.java.in")
            if(NOT "${ARG_JAVA_PARSER_HARNESS}" STREQUAL "")
                set(_java_parser_harness "${ARG_JAVA_PARSER_HARNESS}")
            endif()
            list(APPEND _cmd
                "-DJAVA_PARSER=TRUE"
                "-DJAVA_HARNESS=${_java_parser_harness}")
        endif()
    endif()
    if(ARG_CHECK_PYTHON)
        set(_python_runtime_dir "${LPG2_PYTHON_RUNTIME_DIR}")
        if(NOT "${ARG_RUNTIME_DIR}" STREQUAL "")
            set(_python_runtime_dir "${ARG_RUNTIME_DIR}")
        endif()
        list(APPEND _cmd
            "-DCHECK_PYTHON=TRUE"
            "-DPYTHON_EXECUTABLE=${Python3_EXECUTABLE}"
            "-DPYTHON_RUNTIME_DIR=${_python_runtime_dir}")
        if(ARG_PYTHON_PARSER)
            set(_python_parser_harness
                "${CMAKE_CURRENT_SOURCE_DIR}/harness/python_parser_harness.py.in")
            if(NOT "${ARG_PYTHON_PARSER_HARNESS}" STREQUAL "")
                set(_python_parser_harness "${ARG_PYTHON_PARSER_HARNESS}")
            endif()
            list(APPEND _cmd
                "-DPYTHON_PARSER=TRUE"
                "-DPYTHON_HARNESS=${_python_parser_harness}")
        endif()
    endif()
    if(ARG_CHECK_GO)
        set(_go_runtime_dir "${LPG2_GO_RUNTIME_DIR}")
        if(NOT "${ARG_RUNTIME_DIR}" STREQUAL "")
            set(_go_runtime_dir "${ARG_RUNTIME_DIR}")
        endif()
        list(APPEND _cmd
            "-DCHECK_GO=TRUE"
            "-DGO_EXECUTABLE=${GO_EXECUTABLE}"
            "-DGO_RUNTIME_DIR=${_go_runtime_dir}")
        if(ARG_GO_PARSER)
            set(_go_parser_harness
                "${CMAKE_CURRENT_SOURCE_DIR}/harness/go_parser_harness.go.in")
            if(NOT "${ARG_GO_PARSER_HARNESS}" STREQUAL "")
                set(_go_parser_harness "${ARG_GO_PARSER_HARNESS}")
            endif()
            list(APPEND _cmd
                "-DGO_PARSER=TRUE"
                "-DGO_HARNESS=${_go_parser_harness}")
        endif()
    endif()
    if(ARG_CHECK_TYPESCRIPT)
        set(_ts_runtime_dir "${LPG2_TYPESCRIPT_RUNTIME_DIR}")
        if(NOT "${ARG_RUNTIME_DIR}" STREQUAL "")
            set(_ts_runtime_dir "${ARG_RUNTIME_DIR}")
        endif()
        list(APPEND _cmd
            "-DCHECK_TYPESCRIPT=TRUE"
            "-DNODE_EXECUTABLE=${NODE_EXECUTABLE}"
            "-DNPM_EXECUTABLE=${NPM_EXECUTABLE}"
            "-DTYPESCRIPT_RUNTIME_DIR=${_ts_runtime_dir}")
        if(ARG_TYPESCRIPT_PARSER)
            set(_ts_parser_harness
                "${CMAKE_CURRENT_SOURCE_DIR}/harness/typescript_parser_harness.ts.in")
            if(NOT "${ARG_TYPESCRIPT_PARSER_HARNESS}" STREQUAL "")
                set(_ts_parser_harness "${ARG_TYPESCRIPT_PARSER_HARNESS}")
            endif()
            list(APPEND _cmd
                "-DTYPESCRIPT_PARSER=TRUE"
                "-DTYPESCRIPT_HARNESS=${_ts_parser_harness}")
        endif()
    endif()
    if(ARG_CHECK_CSHARP)
        set(_csharp_runtime_dir "${LPG2_CSHARP_RUNTIME_DIR}")
        if(NOT "${ARG_RUNTIME_DIR}" STREQUAL "")
            set(_csharp_runtime_dir "${ARG_RUNTIME_DIR}")
        endif()
        list(APPEND _cmd
            "-DCHECK_CSHARP=TRUE"
            "-DDOTNET_EXECUTABLE=${DOTNET_EXECUTABLE}"
            "-DCSHARP_RUNTIME_DIR=${_csharp_runtime_dir}")
        if(ARG_CSHARP_PARSER)
            set(_csharp_parser_harness
                "${CMAKE_CURRENT_SOURCE_DIR}/harness/csharp_parser_harness.cs.in")
            if(NOT "${ARG_CSHARP_PARSER_HARNESS}" STREQUAL "")
                set(_csharp_parser_harness "${ARG_CSHARP_PARSER_HARNESS}")
            endif()
            list(APPEND _cmd
                "-DCSHARP_PARSER=TRUE"
                "-DCSHARP_HARNESS=${_csharp_parser_harness}")
        endif()
    endif()
    if(ARG_CHECK_DART)
        set(_dart_runtime_dir "${LPG2_DART_RUNTIME_DIR}")
        if(NOT "${ARG_RUNTIME_DIR}" STREQUAL "")
            set(_dart_runtime_dir "${ARG_RUNTIME_DIR}")
        endif()
        list(APPEND _cmd
            "-DCHECK_DART=TRUE"
            "-DDART_EXECUTABLE=${DART_EXECUTABLE}"
            "-DDART_RUNTIME_DIR=${_dart_runtime_dir}")
        if(ARG_DART_PARSER)
            set(_dart_parser_harness
                "${CMAKE_CURRENT_SOURCE_DIR}/harness/dart_parser_harness.dart.in")
            if(NOT "${ARG_DART_PARSER_HARNESS}" STREQUAL "")
                set(_dart_parser_harness "${ARG_DART_PARSER_HARNESS}")
            endif()
            list(APPEND _cmd
                "-DDART_PARSER=TRUE"
                "-DDART_HARNESS=${_dart_parser_harness}")
        endif()
    endif()
    list(APPEND _cmd -P "${LPG2_TEST_RUNNER}")

    add_test(NAME ${NAME} COMMAND ${_cmd})
    if(ARG_LABELS)
        set_tests_properties(${NAME} PROPERTIES LABELS "${ARG_LABELS}")
    endif()
    # cargo and nested cpplpg2 builds contend on shared caches when ctest runs in parallel.
    if(ARG_CHECK_RUST OR ARG_CPP_PARSER OR ARG_JAVA_PARSER OR ARG_PYTHON_PARSER
            OR ARG_GO_PARSER OR ARG_TYPESCRIPT_PARSER OR ARG_CSHARP_PARSER
            OR ARG_DART_PARSER)
        set_tests_properties(${NAME} PROPERTIES RUN_SERIAL TRUE)
    endif()
endfunction()

# --- Tier A: fast multi-backend smoke (minimal.g) ---
set(_minimal "${LPG2_FIXTURES_DIR}/minimal.g")
foreach(_lang IN ITEMS cpp java go python3 csharp typescript dart rust)
    lpg2_add_generation_test(minimal_${_lang} "${_minimal}" ${_lang} LABELS smoke)
endforeach()

# Compile and run generated tables instead of only checking file existence.
# Use cpp_legacy (CppTable/.cpp) so the table harness stays self-contained
# without LPG-cpp-runtime; user-facing cpp/rt_cpp remain CppTable2.
lpg2_add_generation_test(generated_cpp_compile_run "${_minimal}" cpp_legacy
    CHECK_CPP CPP_PARSE LABELS compile integration parser)

set(_rust_tests_available FALSE)
if(CARGO_EXECUTABLE AND EXISTS "${LPG2_RUST_TEST_RUNTIME_DIR}/Cargo.toml")
    set(_rust_tests_available TRUE)
    lpg2_add_generation_test(generated_rust_compile_run "${_minimal}" rust
        CHECK_RUST RUST_PARSE
        RUNTIME_DIR "${LPG2_RUST_TEST_RUNTIME_DIR}"
        LABELS compile integration parser)
elseif(LPG2_REQUIRE_RUST_TESTS)
    message(FATAL_ERROR
        "Rust table tests require cargo and the test runtime shim "
        "(currently '${LPG2_RUST_TEST_RUNTIME_DIR}')")
else()
    message(STATUS
        "Rust compile/run test disabled: install cargo")
endif()

# Exercise the backtracking table layout through both compilers as well.
set(_backtrack "${LPG2_FIXTURES_DIR}/backtrack.g")
lpg2_add_generation_test(backtrack_cpp_compile_run "${_backtrack}" cpp_legacy
    CHECK_CPP CPP_PARSE LABELS compile integration backtracking parser)
if(_rust_tests_available)
    lpg2_add_generation_test(backtrack_rust_compile_run "${_backtrack}" rust
        CHECK_RUST RUST_PARSE
        RUNTIME_DIR "${LPG2_RUST_TEST_RUNTIME_DIR}"
        LABELS compile integration backtracking parser)
endif()

# Generation smoke for backtracking tables across full backends (file presence).
foreach(_lang IN ITEMS java go python3 csharp typescript dart)
    lpg2_add_generation_test(backtrack_${_lang} "${_backtrack}" ${_lang}
        LABELS smoke backtracking)
endforeach()

# All full backends must at least emit GLR conflict tables and nextAst AST
# scaffolding (via a parser template so Action/AST files are written).
# Only Java currently ships a GLR runtime driver.
set(_glr_templates_root
    "${CMAKE_SOURCE_DIR}/../lpg-generator-templates-2.1.00")
set(_glr_smoke_langs
    "cpp|rt_cpp|dtParserTemplateF.gi"
    "java|java|glrParserTemplateF.gi"
    "go|go|dtParserTemplateF.gi"
    "python3|python3|dtParserTemplateF.gi"
    "csharp|csharp|dtParserTemplateF.gi"
    "typescript|typescript|dtParserTemplateF.gi"
    "dart|dart|dtParserTemplateF.gi"
    "rust|rust|dtParserTemplateF.gi")
foreach(_entry IN LISTS _glr_smoke_langs)
    string(REPLACE "|" ";" _parts "${_entry}")
    list(GET _parts 0 _lang)
    list(GET _parts 1 _tmpl_lang)
    list(GET _parts 2 _tmpl_file)
    lpg2_add_generation_test(glr_${_lang}_smoke
        "${LPG2_FIXTURES_DIR}/glr_expr.g" ${_lang}
        CHECK_GLR_NEXTAST
        TEMPLATE "${_glr_templates_root}/templates/${_tmpl_lang}/${_tmpl_file}"
        INCLUDE_DIR "${_glr_templates_root}/include/${_tmpl_lang}"
        LABELS smoke glr ast)
endforeach()

set(_rust_template_dir
    "${CMAKE_SOURCE_DIR}/../lpg-generator-templates-2.1.00/templates/rust")
set(_rust_include_dir
    "${CMAKE_SOURCE_DIR}/../lpg-generator-templates-2.1.00/include/rust")
set(_rust_parser_tests_available FALSE)
if(CARGO_EXECUTABLE AND EXISTS "${LPG2_RUST_RUNTIME_DIR}/Cargo.toml")
    set(_rust_parser_tests_available TRUE)
    lpg2_add_generation_test(rust_deterministic_parser_run "${_minimal}" rust
        CHECK_RUST RUST_PARSER
        RUNTIME_DIR "${LPG2_RUST_RUNTIME_DIR}"
        TEMPLATE "${_rust_template_dir}/dtParserTemplateF.gi"
        INCLUDE_DIR "${_rust_include_dir}"
        LABELS compile integration parser rust)
    lpg2_add_generation_test(rust_backtracking_parser_run "${_backtrack}" rust
        CHECK_RUST RUST_PARSER
        RUNTIME_DIR "${LPG2_RUST_RUNTIME_DIR}"
        TEMPLATE "${_rust_template_dir}/btParserTemplateF.gi"
        INCLUDE_DIR "${_rust_include_dir}"
        LABELS compile integration parser backtracking rust)
elseif(LPG2_REQUIRE_RUST_PARSER_TESTS)
    message(FATAL_ERROR
        "Full Rust parser tests require LPG2_RUST_RUNTIME_DIR "
        "(currently '${LPG2_RUST_RUNTIME_DIR}')")
else()
    message(STATUS
        "Full Rust parser tests disabled: set LPG2_RUST_RUNTIME_DIR")
endif()

if(_rust_parser_tests_available)
    lpg2_add_generation_test(rust_automatic_ast_nested_behavior
        "${LPG2_FIXTURES_DIR}/ast_nested.g" rust
        CHECK_RUST RUST_PARSER
        RUNTIME_DIR "${LPG2_RUST_RUNTIME_DIR}"
        TEMPLATE "${_rust_template_dir}/dtParserTemplateF.gi"
        INCLUDE_DIR "${_rust_include_dir}"
        RUST_PARSER_HARNESS
            "${CMAKE_CURRENT_SOURCE_DIR}/harness/rust_parser_harness_nested.rs.in"
        LABELS compile integration parser rust ast)
    lpg2_add_generation_test(rust_automatic_ast_list_behavior
        "${LPG2_FIXTURES_DIR}/ast_list.g" rust
        CHECK_RUST RUST_PARSER
        RUNTIME_DIR "${LPG2_RUST_RUNTIME_DIR}"
        TEMPLATE "${_rust_template_dir}/dtParserTemplateF.gi"
        INCLUDE_DIR "${_rust_include_dir}"
        RUST_PARSER_HARNESS
            "${CMAKE_CURRENT_SOURCE_DIR}/harness/rust_parser_harness_list.rs.in"
        LABELS compile integration parser rust ast)
    lpg2_add_generation_test(rust_automatic_ast_parent_behavior
        "${LPG2_FIXTURES_DIR}/ast_parent.g" rust
        CHECK_RUST RUST_PARSER
        RUNTIME_DIR "${LPG2_RUST_RUNTIME_DIR}"
        TEMPLATE "${_rust_template_dir}/dtParserTemplateF.gi"
        INCLUDE_DIR "${_rust_include_dir}"
        RUST_PARSER_HARNESS
            "${CMAKE_CURRENT_SOURCE_DIR}/harness/rust_parser_harness_parent.rs.in"
        LABELS compile integration parser rust ast)
    lpg2_add_generation_test(rust_automatic_ast_preorder_behavior
        "${LPG2_FIXTURES_DIR}/ast_preorder.g" rust
        CHECK_RUST RUST_PARSER
        RUNTIME_DIR "${LPG2_RUST_RUNTIME_DIR}"
        TEMPLATE "${_rust_template_dir}/dtParserTemplateF.gi"
        INCLUDE_DIR "${_rust_include_dir}"
        RUST_PARSER_HARNESS
            "${CMAKE_CURRENT_SOURCE_DIR}/harness/rust_parser_harness_preorder.rs.in"
        LABELS compile integration parser rust ast)
    lpg2_add_generation_test(rust_automatic_ast_env_behavior
        "${LPG2_FIXTURES_DIR}/ast_env.g" rust
        CHECK_RUST RUST_PARSER
        RUNTIME_DIR "${LPG2_RUST_RUNTIME_DIR}"
        TEMPLATE "${_rust_template_dir}/dtParserTemplateF.gi"
        INCLUDE_DIR "${_rust_include_dir}"
        RUST_PARSER_HARNESS
            "${CMAKE_CURRENT_SOURCE_DIR}/harness/rust_parser_harness_env.rs.in"
        LABELS compile integration parser rust ast)
    lpg2_add_generation_test(rust_automatic_ast_iface_behavior
        "${LPG2_FIXTURES_DIR}/ast_iface.g" rust
        CHECK_RUST RUST_PARSER
        RUNTIME_DIR "${LPG2_RUST_RUNTIME_DIR}"
        TEMPLATE "${_rust_template_dir}/dtParserTemplateF.gi"
        INCLUDE_DIR "${_rust_include_dir}"
        RUST_PARSER_HARNESS
            "${CMAKE_CURRENT_SOURCE_DIR}/harness/rust_parser_harness_iface.rs.in"
        LABELS compile integration parser rust ast)
    lpg2_add_generation_test(rust_automatic_ast_recover
        "${LPG2_FIXTURES_DIR}/ast_recover_noblock.g" rust
        CHECK_RUST RUST_PARSER
        RUNTIME_DIR "${LPG2_RUST_RUNTIME_DIR}"
        TEMPLATE "${_rust_template_dir}/dtParserTemplateF.gi"
        INCLUDE_DIR "${_rust_include_dir}"
        RUST_PARSER_HARNESS
            "${CMAKE_CURRENT_SOURCE_DIR}/harness/rust_parser_harness_recover.rs.in"
        LABELS compile integration parser rust ast recover)
    lpg2_add_generation_test(rust_expected_tokens
        "${LPG2_FIXTURES_DIR}/ast_nested.g" rust
        CHECK_RUST RUST_PARSER
        RUNTIME_DIR "${LPG2_RUST_RUNTIME_DIR}"
        TEMPLATE "${_rust_template_dir}/dtParserTemplateF.gi"
        INCLUDE_DIR "${_rust_include_dir}"
        RUST_PARSER_HARNESS
            "${CMAKE_CURRENT_SOURCE_DIR}/harness/rust_parser_harness_expected_tokens.rs.in"
        LABELS compile integration parser rust expected-tokens)
else()
    message(STATUS
        "Rust automatic AST test disabled: set LPG2_RUST_RUNTIME_DIR")
endif()

set(_cpp_template_dir
    "${CMAKE_SOURCE_DIR}/../lpg-generator-templates-2.1.00/templates/rt_cpp")
set(_cpp_include_dir
    "${CMAKE_SOURCE_DIR}/../lpg-generator-templates-2.1.00/include/rt_cpp")
if(EXISTS "${LPG2_CPP_RUNTIME_DIR}/CMakeLists.txt")
    lpg2_add_generation_test(cpp_automatic_ast_nested
        "${LPG2_FIXTURES_DIR}/ast_nested.g" rt_cpp
        CHECK_CPP CPP_PARSER
        RUNTIME_DIR "${LPG2_CPP_RUNTIME_DIR}"
        TEMPLATE "${_cpp_template_dir}/dtParserTemplateF.gi"
        INCLUDE_DIR "${_cpp_include_dir}"
        LABELS compile integration parser cpp ast)
    lpg2_add_generation_test(cpp_automatic_ast_list
        "${LPG2_FIXTURES_DIR}/ast_list.g" rt_cpp
        CHECK_CPP CPP_PARSER
        CPP_PARSER_HARNESS
            "${CMAKE_CURRENT_SOURCE_DIR}/harness/cpp_parser_harness_list.cpp.in"
        RUNTIME_DIR "${LPG2_CPP_RUNTIME_DIR}"
        TEMPLATE "${_cpp_template_dir}/dtParserTemplateF.gi"
        INCLUDE_DIR "${_cpp_include_dir}"
        LABELS compile integration parser cpp ast)
    lpg2_add_generation_test(cpp_expected_tokens
        "${LPG2_FIXTURES_DIR}/ast_nested.g" rt_cpp
        CHECK_CPP CPP_PARSER
        CPP_PARSER_HARNESS
            "${CMAKE_CURRENT_SOURCE_DIR}/harness/cpp_parser_harness_expected_tokens.cpp.in"
        RUNTIME_DIR "${LPG2_CPP_RUNTIME_DIR}"
        TEMPLATE "${_cpp_template_dir}/dtParserTemplateF.gi"
        INCLUDE_DIR "${_cpp_include_dir}"
        LABELS compile integration parser cpp expected-tokens)
    # Prosthetic-AST closed loop: %Recover nonterminal -> getProstheticAst()
    # factory reachable via getProsthesisIndex(kind), synthesizing a placeholder
    # node instead of throwing during backtracking recovery.
    lpg2_add_generation_test(cpp_automatic_ast_recover
        "${LPG2_FIXTURES_DIR}/ast_recover_noblock.g" rt_cpp
        CHECK_CPP CPP_PARSER
        CPP_PARSER_HARNESS
            "${CMAKE_CURRENT_SOURCE_DIR}/harness/cpp_parser_harness_recover.cpp.in"
        RUNTIME_DIR "${LPG2_CPP_RUNTIME_DIR}"
        TEMPLATE "${_cpp_template_dir}/dtParserTemplateF.gi"
        INCLUDE_DIR "${_cpp_include_dir}"
        LABELS compile integration parser cpp ast recover)
    # Token-stream damage reset + re-seed + re-parse (not tree-sitter reuse).
    lpg2_add_generation_test(cpp_automatic_ast_incremental
        "${LPG2_FIXTURES_DIR}/ast_nested.g" rt_cpp
        CHECK_CPP CPP_PARSER
        CPP_PARSER_HARNESS
            "${CMAKE_CURRENT_SOURCE_DIR}/harness/cpp_parser_harness_incremental.cpp.in"
        RUNTIME_DIR "${LPG2_CPP_RUNTIME_DIR}"
        TEMPLATE "${_cpp_template_dir}/dtParserTemplateF.gi"
        INCLUDE_DIR "${_cpp_include_dir}"
        LABELS compile integration parser cpp ast incremental)
else()
    if(LPG2_REQUIRE_CPP_PARSER_TESTS)
        message(FATAL_ERROR
            "C++ parser/AST tests require LPG2_CPP_RUNTIME_DIR "
            "(currently '${LPG2_CPP_RUNTIME_DIR}')")
    else()
        message(STATUS
            "C++ automatic AST test disabled: set LPG2_CPP_RUNTIME_DIR")
    endif()
endif()

set(_java_template_dir
    "${CMAKE_SOURCE_DIR}/../lpg-generator-templates-2.1.00/templates/java")
set(_java_include_dir
    "${CMAKE_SOURCE_DIR}/../lpg-generator-templates-2.1.00/include/java")
set(_java_parser_tests_available FALSE)
if(Java_JAVAC_EXECUTABLE AND Java_JAVA_EXECUTABLE
        AND EXISTS "${LPG2_JAVA_RUNTIME_DIR}/src/lpg/runtime")
    set(_java_parser_tests_available TRUE)
    lpg2_add_generation_test(java_automatic_ast_nested
        "${LPG2_FIXTURES_DIR}/ast_nested.g" java
        CHECK_JAVA JAVA_PARSER
        RUNTIME_DIR "${LPG2_JAVA_RUNTIME_DIR}"
        TEMPLATE "${_java_template_dir}/dtParserTemplateF.gi"
        INCLUDE_DIR "${_java_include_dir}"
        LABELS compile integration parser java ast)
    lpg2_add_generation_test(java_automatic_ast_list
        "${LPG2_FIXTURES_DIR}/ast_list.g" java
        CHECK_JAVA JAVA_PARSER
        JAVA_PARSER_HARNESS
            "${CMAKE_CURRENT_SOURCE_DIR}/harness/java_parser_harness_list.java.in"
        RUNTIME_DIR "${LPG2_JAVA_RUNTIME_DIR}"
        TEMPLATE "${_java_template_dir}/dtParserTemplateF.gi"
        INCLUDE_DIR "${_java_include_dir}"
        LABELS compile integration parser java ast)
    # Prosthetic-AST closed loop: %Recover nonterminal -> getProstheticAst()
    # factory reachable via getProsthesisIndex(kind), synthesizing a placeholder
    # node instead of throwing during backtracking recovery.
    lpg2_add_generation_test(java_automatic_ast_recover
        "${LPG2_FIXTURES_DIR}/ast_recover_noblock.g" java
        CHECK_JAVA JAVA_PARSER
        JAVA_PARSER_HARNESS
            "${CMAKE_CURRENT_SOURCE_DIR}/harness/java_parser_harness_recover.java.in"
        RUNTIME_DIR "${LPG2_JAVA_RUNTIME_DIR}"
        TEMPLATE "${_java_template_dir}/dtParserTemplateF.gi"
        INCLUDE_DIR "${_java_include_dir}"
        LABELS compile integration parser java ast recover)
    lpg2_add_generation_test(java_expected_tokens
        "${LPG2_FIXTURES_DIR}/ast_nested.g" java
        CHECK_JAVA JAVA_PARSER
        JAVA_PARSER_HARNESS
            "${CMAKE_CURRENT_SOURCE_DIR}/harness/java_parser_harness_expected_tokens.java.in"
        RUNTIME_DIR "${LPG2_JAVA_RUNTIME_DIR}"
        TEMPLATE "${_java_template_dir}/dtParserTemplateF.gi"
        INCLUDE_DIR "${_java_include_dir}"
        LABELS compile integration parser java expected-tokens)
    lpg2_add_generation_test(java_glr_ambiguous_e2e
        "${LPG2_FIXTURES_DIR}/glr_expr.g" java
        CHECK_JAVA JAVA_PARSER
        JAVA_PARSER_HARNESS
            "${CMAKE_CURRENT_SOURCE_DIR}/harness/java_parser_harness_glr.java.in"
        RUNTIME_DIR "${LPG2_JAVA_RUNTIME_DIR}"
        TEMPLATE "${_java_template_dir}/glrParserTemplateF.gi"
        INCLUDE_DIR "${_java_include_dir}"
        LABELS compile integration parser java ast glr)
    lpg2_add_generation_test(java_glr_entry_e2e
        "${LPG2_FIXTURES_DIR}/glr_entry.g" java
        CHECK_JAVA JAVA_PARSER
        JAVA_PARSER_HARNESS
            "${CMAKE_CURRENT_SOURCE_DIR}/harness/java_parser_harness_glr_entry.java.in"
        RUNTIME_DIR "${LPG2_JAVA_RUNTIME_DIR}"
        TEMPLATE "${_java_template_dir}/glrParserTemplateF.gi"
        INCLUDE_DIR "${_java_include_dir}"
        LABELS compile integration parser java ast glr entry)
    lpg2_add_generation_test(java_glr_rr_epsilon_e2e
        "${LPG2_FIXTURES_DIR}/glr_rr_epsilon.g" java
        CHECK_JAVA JAVA_PARSER
        JAVA_PARSER_HARNESS
            "${CMAKE_CURRENT_SOURCE_DIR}/harness/java_parser_harness_glr_rr_epsilon.java.in"
        RUNTIME_DIR "${LPG2_JAVA_RUNTIME_DIR}"
        TEMPLATE "${_java_template_dir}/glrParserTemplateF.gi"
        INCLUDE_DIR "${_java_include_dir}"
        LABELS compile integration parser java ast glr rr epsilon)
    lpg2_add_generation_test(java_glr_correlation_e2e
        "${LPG2_FIXTURES_DIR}/glr_correlation.g" java
        CHECK_JAVA JAVA_PARSER
        JAVA_PARSER_HARNESS
            "${CMAKE_CURRENT_SOURCE_DIR}/harness/java_parser_harness_glr_correlation.java.in"
        RUNTIME_DIR "${LPG2_JAVA_RUNTIME_DIR}"
        TEMPLATE "${_java_template_dir}/glrParserTemplateF.gi"
        INCLUDE_DIR "${_java_include_dir}"
        LABELS compile integration parser java ast glr correlation)
    lpg2_add_generation_test(java_glr_symbol_identity_e2e
        "${LPG2_FIXTURES_DIR}/glr_symbol_identity.g" java
        CHECK_JAVA JAVA_PARSER
        JAVA_PARSER_HARNESS
            "${CMAKE_CURRENT_SOURCE_DIR}/harness/java_parser_harness_glr_symbol_identity.java.in"
        RUNTIME_DIR "${LPG2_JAVA_RUNTIME_DIR}"
        TEMPLATE "${_java_template_dir}/glrParserTemplateF.gi"
        INCLUDE_DIR "${_java_include_dir}"
        LABELS compile integration parser java ast glr symbol-identity)
elseif(LPG2_REQUIRE_JAVA_PARSER_TESTS)
    message(FATAL_ERROR
        "Java parser/AST tests require javac/java and LPG2_JAVA_RUNTIME_DIR "
        "(currently '${LPG2_JAVA_RUNTIME_DIR}')")
else()
    message(STATUS
        "Java automatic AST test disabled: install JDK and set LPG2_JAVA_RUNTIME_DIR")
endif()

set(_python_template_dir
    "${CMAKE_SOURCE_DIR}/../lpg-generator-templates-2.1.00/templates/python3")
set(_python_include_dir
    "${CMAKE_SOURCE_DIR}/../lpg-generator-templates-2.1.00/include/python3")
if(Python3_EXECUTABLE AND EXISTS "${LPG2_PYTHON_RUNTIME_DIR}/lpg2")
    lpg2_add_generation_test(python_automatic_ast_nested
        "${LPG2_FIXTURES_DIR}/ast_nested.g" python3
        CHECK_PYTHON PYTHON_PARSER
        RUNTIME_DIR "${LPG2_PYTHON_RUNTIME_DIR}"
        TEMPLATE "${_python_template_dir}/dtParserTemplateF.gi"
        INCLUDE_DIR "${_python_include_dir}"
        LABELS compile integration parser python ast)
    lpg2_add_generation_test(python_automatic_ast_recover
        "${LPG2_FIXTURES_DIR}/ast_recover_noblock.g" python3
        CHECK_PYTHON PYTHON_PARSER
        RUNTIME_DIR "${LPG2_PYTHON_RUNTIME_DIR}"
        TEMPLATE "${_python_template_dir}/dtParserTemplateF.gi"
        INCLUDE_DIR "${_python_include_dir}"
        PYTHON_PARSER_HARNESS
            "${CMAKE_CURRENT_SOURCE_DIR}/harness/python_parser_harness_recover.py.in"
        LABELS compile integration parser python ast recover)
    lpg2_add_generation_test(python_expected_tokens
        "${LPG2_FIXTURES_DIR}/ast_nested.g" python3
        CHECK_PYTHON PYTHON_PARSER
        RUNTIME_DIR "${LPG2_PYTHON_RUNTIME_DIR}"
        TEMPLATE "${_python_template_dir}/dtParserTemplateF.gi"
        INCLUDE_DIR "${_python_include_dir}"
        PYTHON_PARSER_HARNESS
            "${CMAKE_CURRENT_SOURCE_DIR}/harness/python_parser_harness_expected_tokens.py.in"
        LABELS compile integration parser python expected-tokens)
elseif(LPG2_REQUIRE_PYTHON_PARSER_TESTS)
    message(FATAL_ERROR
        "Python parser/AST tests require python3 and LPG2_PYTHON_RUNTIME_DIR "
        "(currently '${LPG2_PYTHON_RUNTIME_DIR}')")
else()
    message(STATUS
        "Python automatic AST test disabled: install python3 and set LPG2_PYTHON_RUNTIME_DIR")
endif()

set(_go_template_dir
    "${CMAKE_SOURCE_DIR}/../lpg-generator-templates-2.1.00/templates/go")
set(_go_include_dir
    "${CMAKE_SOURCE_DIR}/../lpg-generator-templates-2.1.00/include/go")
if(GO_EXECUTABLE AND EXISTS "${LPG2_GO_RUNTIME_DIR}/go.mod")
    lpg2_add_generation_test(go_automatic_ast_nested
        "${LPG2_FIXTURES_DIR}/ast_nested.g" go
        CHECK_GO GO_PARSER
        RUNTIME_DIR "${LPG2_GO_RUNTIME_DIR}"
        TEMPLATE "${_go_template_dir}/dtParserTemplateF.gi"
        INCLUDE_DIR "${_go_include_dir}"
        LABELS compile integration parser go ast)
    lpg2_add_generation_test(go_automatic_ast_recover
        "${LPG2_FIXTURES_DIR}/ast_recover_noblock.g" go
        CHECK_GO GO_PARSER
        RUNTIME_DIR "${LPG2_GO_RUNTIME_DIR}"
        TEMPLATE "${_go_template_dir}/dtParserTemplateF.gi"
        INCLUDE_DIR "${_go_include_dir}"
        GO_PARSER_HARNESS
            "${CMAKE_CURRENT_SOURCE_DIR}/harness/go_parser_harness_recover.go.in"
        LABELS compile integration parser go ast recover)
    lpg2_add_generation_test(go_expected_tokens
        "${LPG2_FIXTURES_DIR}/ast_nested.g" go
        CHECK_GO GO_PARSER
        RUNTIME_DIR "${LPG2_GO_RUNTIME_DIR}"
        TEMPLATE "${_go_template_dir}/dtParserTemplateF.gi"
        INCLUDE_DIR "${_go_include_dir}"
        GO_PARSER_HARNESS
            "${CMAKE_CURRENT_SOURCE_DIR}/harness/go_parser_harness_expected_tokens.go.in"
        LABELS compile integration parser go expected-tokens)
elseif(LPG2_REQUIRE_GO_PARSER_TESTS)
    message(FATAL_ERROR
        "Go parser/AST tests require go and LPG2_GO_RUNTIME_DIR "
        "(currently '${LPG2_GO_RUNTIME_DIR}')")
else()
    message(STATUS
        "Go automatic AST test disabled: install go and set LPG2_GO_RUNTIME_DIR")
endif()

set(_typescript_template_dir
    "${CMAKE_SOURCE_DIR}/../lpg-generator-templates-2.1.00/templates/typescript")
set(_typescript_include_dir
    "${CMAKE_SOURCE_DIR}/../lpg-generator-templates-2.1.00/include/typescript")
if(NODE_EXECUTABLE AND NPM_EXECUTABLE
        AND EXISTS "${LPG2_TYPESCRIPT_RUNTIME_DIR}/package.json")
    lpg2_add_generation_test(typescript_automatic_ast_nested
        "${LPG2_FIXTURES_DIR}/ast_nested.g" typescript
        CHECK_TYPESCRIPT TYPESCRIPT_PARSER
        RUNTIME_DIR "${LPG2_TYPESCRIPT_RUNTIME_DIR}"
        TEMPLATE "${_typescript_template_dir}/dtParserTemplateF.gi"
        INCLUDE_DIR "${_typescript_include_dir}"
        LABELS compile integration parser typescript ast)
    lpg2_add_generation_test(typescript_expected_tokens
        "${LPG2_FIXTURES_DIR}/ast_nested.g" typescript
        CHECK_TYPESCRIPT TYPESCRIPT_PARSER
        TYPESCRIPT_PARSER_HARNESS
            "${CMAKE_CURRENT_SOURCE_DIR}/harness/typescript_parser_harness_expected_tokens.ts.in"
        RUNTIME_DIR "${LPG2_TYPESCRIPT_RUNTIME_DIR}"
        TEMPLATE "${_typescript_template_dir}/dtParserTemplateF.gi"
        INCLUDE_DIR "${_typescript_include_dir}"
        LABELS compile integration parser typescript expected-tokens)
    lpg2_add_generation_test(typescript_automatic_ast_recover
        "${LPG2_FIXTURES_DIR}/ast_recover_noblock.g" typescript
        CHECK_TYPESCRIPT TYPESCRIPT_PARSER
        RUNTIME_DIR "${LPG2_TYPESCRIPT_RUNTIME_DIR}"
        TEMPLATE "${_typescript_template_dir}/dtParserTemplateF.gi"
        INCLUDE_DIR "${_typescript_include_dir}"
        TYPESCRIPT_PARSER_HARNESS
            "${CMAKE_CURRENT_SOURCE_DIR}/harness/typescript_parser_harness_recover.ts.in"
        LABELS compile integration parser typescript ast recover)
elseif(LPG2_REQUIRE_TYPESCRIPT_PARSER_TESTS)
    message(FATAL_ERROR
        "TypeScript parser/AST tests require node/npm and "
        "LPG2_TYPESCRIPT_RUNTIME_DIR (currently '${LPG2_TYPESCRIPT_RUNTIME_DIR}')")
else()
    message(STATUS
        "TypeScript automatic AST test disabled: install node/npm and set "
        "LPG2_TYPESCRIPT_RUNTIME_DIR")
endif()

set(_csharp_template_dir
    "${CMAKE_SOURCE_DIR}/../lpg-generator-templates-2.1.00/templates/csharp")
set(_csharp_include_dir
    "${CMAKE_SOURCE_DIR}/../lpg-generator-templates-2.1.00/include/csharp")
if(DOTNET_EXECUTABLE
        AND EXISTS "${LPG2_CSHARP_RUNTIME_DIR}/LPG2.Runtime.csproj")
    lpg2_add_generation_test(csharp_automatic_ast_nested
        "${LPG2_FIXTURES_DIR}/ast_nested.g" csharp
        CHECK_CSHARP CSHARP_PARSER
        RUNTIME_DIR "${LPG2_CSHARP_RUNTIME_DIR}"
        TEMPLATE "${_csharp_template_dir}/dtParserTemplateF.gi"
        INCLUDE_DIR "${_csharp_include_dir}"
        LABELS compile integration parser csharp ast)
    lpg2_add_generation_test(csharp_automatic_ast_recover
        "${LPG2_FIXTURES_DIR}/ast_recover_noblock.g" csharp
        CHECK_CSHARP CSHARP_PARSER
        RUNTIME_DIR "${LPG2_CSHARP_RUNTIME_DIR}"
        TEMPLATE "${_csharp_template_dir}/dtParserTemplateF.gi"
        INCLUDE_DIR "${_csharp_include_dir}"
        CSHARP_PARSER_HARNESS
            "${CMAKE_CURRENT_SOURCE_DIR}/harness/csharp_parser_harness_recover.cs.in"
        LABELS compile integration parser csharp ast recover)
    lpg2_add_generation_test(csharp_expected_tokens
        "${LPG2_FIXTURES_DIR}/ast_nested.g" csharp
        CHECK_CSHARP CSHARP_PARSER
        RUNTIME_DIR "${LPG2_CSHARP_RUNTIME_DIR}"
        TEMPLATE "${_csharp_template_dir}/dtParserTemplateF.gi"
        INCLUDE_DIR "${_csharp_include_dir}"
        CSHARP_PARSER_HARNESS
            "${CMAKE_CURRENT_SOURCE_DIR}/harness/csharp_parser_harness_expected_tokens.cs.in"
        LABELS compile integration parser csharp expected-tokens)
elseif(LPG2_REQUIRE_CSHARP_PARSER_TESTS)
    message(FATAL_ERROR
        "C# parser/AST tests require dotnet and LPG2_CSHARP_RUNTIME_DIR "
        "(currently '${LPG2_CSHARP_RUNTIME_DIR}')")
else()
    message(STATUS
        "C# automatic AST test disabled: install dotnet and set "
        "LPG2_CSHARP_RUNTIME_DIR")
endif()

set(_dart_template_dir
    "${CMAKE_SOURCE_DIR}/../lpg-generator-templates-2.1.00/templates/dart")
set(_dart_include_dir
    "${CMAKE_SOURCE_DIR}/../lpg-generator-templates-2.1.00/include/dart")
if(DART_EXECUTABLE AND EXISTS "${LPG2_DART_RUNTIME_DIR}/pubspec.yaml")
    lpg2_add_generation_test(dart_automatic_ast_nested
        "${LPG2_FIXTURES_DIR}/ast_nested.g" dart
        CHECK_DART DART_PARSER
        RUNTIME_DIR "${LPG2_DART_RUNTIME_DIR}"
        TEMPLATE "${_dart_template_dir}/dtParserTemplateF.gi"
        INCLUDE_DIR "${_dart_include_dir}"
        LABELS compile integration parser dart ast)
    lpg2_add_generation_test(dart_automatic_ast_recover
        "${LPG2_FIXTURES_DIR}/ast_recover_noblock.g" dart
        CHECK_DART DART_PARSER
        RUNTIME_DIR "${LPG2_DART_RUNTIME_DIR}"
        TEMPLATE "${_dart_template_dir}/dtParserTemplateF.gi"
        INCLUDE_DIR "${_dart_include_dir}"
        DART_PARSER_HARNESS
            "${CMAKE_CURRENT_SOURCE_DIR}/harness/dart_parser_harness_recover.dart.in"
        LABELS compile integration parser dart ast recover)
    lpg2_add_generation_test(dart_expected_tokens
        "${LPG2_FIXTURES_DIR}/ast_nested.g" dart
        CHECK_DART DART_PARSER
        RUNTIME_DIR "${LPG2_DART_RUNTIME_DIR}"
        TEMPLATE "${_dart_template_dir}/dtParserTemplateF.gi"
        INCLUDE_DIR "${_dart_include_dir}"
        DART_PARSER_HARNESS
            "${CMAKE_CURRENT_SOURCE_DIR}/harness/dart_parser_harness_expected_tokens.dart.in"
        LABELS compile integration parser dart expected-tokens)
elseif(LPG2_REQUIRE_DART_PARSER_TESTS)
    message(FATAL_ERROR
        "Dart parser/AST tests require dart and LPG2_DART_RUNTIME_DIR "
        "(currently '${LPG2_DART_RUNTIME_DIR}')")
else()
    message(STATUS
        "Dart automatic AST test disabled: install dart and set "
        "LPG2_DART_RUNTIME_DIR")
endif()

# rt_cpp backtracking / lexer template smoke (generation; parser e2e stays on dt+AST).
lpg2_add_generation_test(cpp_bt_template_smoke
    "${_backtrack}" rt_cpp
    TEMPLATE "${_cpp_template_dir}/btParserTemplateF.gi"
    INCLUDE_DIR "${_cpp_include_dir}"
    LABELS smoke cpp backtracking)
lpg2_add_generation_test(cpp_lexer_template_smoke
    "${_minimal}" rt_cpp
    TEMPLATE "${_cpp_template_dir}/LexerTemplateF.gi"
    INCLUDE_DIR "${_cpp_include_dir}"
    LABELS smoke cpp lexer)

# --- Tier B: self-hosting bootstrap integration ---
set(_jikespg "${CMAKE_SOURCE_DIR}/grammar/jikespg.g")
# jikespg.g emits jikespg_prs.* / jikespg_sym.* (underscore after prefix).
lpg2_add_generation_test(bootstrap_cpp "${_jikespg}" cpp
    PREFIX jikespg_ LABELS integration)
lpg2_add_generation_test(bootstrap_rust "${_jikespg}" rust
    PREFIX jikespg_ LABELS integration)

# Rebuild LPG2 with freshly generated bootstrap sources and require stage two
# to be byte-for-byte stable, including the shift/reduce conflict count.
get_target_property(_lpg2_binary_name lpg2 OUTPUT_NAME)
set(_bootstrap_stage2_cmd
    ${CMAKE_COMMAND}
    "-DLPG2_EXE=$<TARGET_FILE:lpg2>"
    "-DSOURCE_DIR=${CMAKE_SOURCE_DIR}"
    "-DGRAMMAR=${_jikespg}"
    "-DWORK_DIR=${LPG2_CTEST_OUT_ROOT}/bootstrap_stage2"
    "-DBINARY_NAME=${_lpg2_binary_name}"
    "-DEXE_SUFFIX=${CMAKE_EXECUTABLE_SUFFIX}"
    "-DGENERATOR=${CMAKE_GENERATOR}"
    "-DGENERATOR_PLATFORM=${CMAKE_GENERATOR_PLATFORM}"
    "-DGENERATOR_TOOLSET=${CMAKE_GENERATOR_TOOLSET}"
    "-DTEST_CONFIG=$<CONFIG>"
    "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}"
    "-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}")
if(CMAKE_MAKE_PROGRAM)
    list(APPEND _bootstrap_stage2_cmd
        "-DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}")
endif()
list(APPEND _bootstrap_stage2_cmd
    -P "${CMAKE_CURRENT_SOURCE_DIR}/run_bootstrap_test.cmake")
add_test(NAME bootstrap_stage2 COMMAND ${_bootstrap_stage2_cmd})
set_tests_properties(bootstrap_stage2 PROPERTIES
    LABELS "bootstrap;integration"
    RUN_SERIAL TRUE
    TIMEOUT 300)

# --- Tier C: feature / error-path regression ---
lpg2_add_generation_test(dropactions_import
    "${LPG2_FIXTURES_DIR}/import_dropactions.g" cpp
    PREFIX import_dropactions LABELS smoke feature)

set(_sr_conflict "${LPG2_FIXTURES_DIR}/shift_reduce_conflict.g")
lpg2_add_generation_test(conflict_warns_ok
    "${_sr_conflict}" cpp
    EXPECT_MESSAGE "example lookahead"
    LABELS smoke feature)

lpg2_add_generation_test(conflict_fail_fast
    "${_sr_conflict}" cpp
    EXTRA_ARGS -fail_on_conflicts
    EXPECT_EXIT_CODE 12
    EXPECT_MESSAGE "fail_on_conflicts"
    EXPECT_NO_OUTPUT
    LABELS smoke feature)

lpg2_add_generation_test(minimal_cpp_golden
    "${_minimal}" cpp
    CHECK_GOLDEN
    GOLDEN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/golden/minimal/cpp"
    LABELS smoke golden)

lpg2_add_generation_test(minimal_rust_golden
    "${_minimal}" rust
    CHECK_GOLDEN
    GOLDEN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/golden/minimal/rust"
    LABELS smoke golden)

lpg2_add_generation_test(minimal_java_golden
    "${_minimal}" java
    CHECK_GOLDEN
    GOLDEN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/golden/minimal/java"
    LABELS smoke golden)

lpg2_add_generation_test(minimal_go_golden
    "${_minimal}" go
    CHECK_GOLDEN
    GOLDEN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/golden/minimal/go"
    LABELS smoke golden)

lpg2_add_generation_test(minimal_python3_golden
    "${_minimal}" python3
    CHECK_GOLDEN
    GOLDEN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/golden/minimal/python3"
    LABELS smoke golden)

lpg2_add_generation_test(minimal_csharp_golden
    "${_minimal}" csharp
    CHECK_GOLDEN
    GOLDEN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/golden/minimal/csharp"
    LABELS smoke golden)

lpg2_add_generation_test(minimal_typescript_golden
    "${_minimal}" typescript
    CHECK_GOLDEN
    GOLDEN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/golden/minimal/typescript"
    LABELS smoke golden)

lpg2_add_generation_test(minimal_dart_golden
    "${_minimal}" dart
    CHECK_GOLDEN
    GOLDEN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/golden/minimal/dart"
    LABELS smoke golden)

# GLR conflict-table encoding locked against glr_expr.g (ambiguous E ::= E PLUS E).
# Uses btParserTemplateF for generation; golden compares *prs* only.
lpg2_add_generation_test(glr_tables_golden_java
    "${LPG2_FIXTURES_DIR}/glr_expr.g" java
    CHECK_GOLDEN
    GOLDEN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/golden/glr/java"
    TEMPLATE "${CMAKE_SOURCE_DIR}/../lpg-generator-templates-2.1.00/templates/java/btParserTemplateF.gi"
    INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../lpg-generator-templates-2.1.00/include/java"
    LABELS smoke golden glr)

# Ensure glrParserTemplateF.gi expands cleanly (driver template, not table golden).
lpg2_add_generation_test(java_glr_template_expand
    "${LPG2_FIXTURES_DIR}/glr_expr.g" java
    TEMPLATE "${CMAKE_SOURCE_DIR}/../lpg-generator-templates-2.1.00/templates/java/glrParserTemplateF.gi"
    INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../lpg-generator-templates-2.1.00/include/java"
    LABELS smoke glr)

lpg2_add_generation_test(invalid_lang_fails
    "${_minimal}" notalang
    EXPECT_EXIT_CODE 12
    EXPECT_MESSAGE "invalid value"
    EXPECT_NO_OUTPUT
    LABELS smoke feature)

lpg2_add_generation_test(invalid_grammar_fails
    "${LPG2_FIXTURES_DIR}/invalid.g" cpp
    EXPECT_EXIT_CODE 12
    EXPECT_MESSAGE "help:"
    EXPECT_NO_OUTPUT
    LABELS smoke feature)

foreach(_diagnostics_case IN ITEMS bad conflict health glr)
    if(_diagnostics_case STREQUAL "bad")
        set(_diagnostics_grammar "${LPG2_FIXTURES_DIR}/invalid.g")
    elseif(_diagnostics_case STREQUAL "conflict"
            OR _diagnostics_case STREQUAL "glr")
        set(_diagnostics_grammar "${_sr_conflict}")
    else()
        set(_diagnostics_grammar "${LPG2_FIXTURES_DIR}/ast_recover.g")
    endif()
    add_test(
        NAME diagnostics_json_${_diagnostics_case}
        COMMAND
            ${CMAKE_COMMAND}
            "-DLPG2_EXE=$<TARGET_FILE:lpg2>"
            "-DGRAMMAR=${_diagnostics_grammar}"
            "-DWORK_DIR=${LPG2_CTEST_OUT_ROOT}/diagnostics_json_${_diagnostics_case}"
            "-DMODE=${_diagnostics_case}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/run_diagnostics_json_test.cmake")
    set_tests_properties(diagnostics_json_${_diagnostics_case} PROPERTIES
        LABELS "cli;diagnostics;smoke")
endforeach()

lpg2_add_generation_test(missing_include_dir_fails
    "${_minimal}" java
    EXTRA_ARGS "-include-directory=/nonexistent/lpg2_ci_missing_include_xyz"
    EXPECT_EXIT_CODE 12
    EXPECT_MESSAGE "Include directory does not exist"
    EXPECT_NO_OUTPUT
    LABELS smoke feature)

add_test(
    NAME malformed_input_corpus
    COMMAND
        ${CMAKE_COMMAND}
        "-DLPG2_EXE=$<TARGET_FILE:lpg2>"
        "-DOUT_DIR=${LPG2_CTEST_OUT_ROOT}/malformed_input_corpus"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/run_malformed_inputs_test.cmake")
set_tests_properties(malformed_input_corpus PROPERTIES
    LABELS "negative;fuzz;smoke")

add_test(
    NAME cli_contract
    COMMAND
        ${CMAKE_COMMAND}
        "-DLPG2_EXE=$<TARGET_FILE:lpg2>"
        "-DGRAMMAR=${_minimal}"
        "-DOUT_DIR=${LPG2_CTEST_OUT_ROOT}/cli_contract"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/run_cli_test.cmake")
set_tests_properties(cli_contract PROPERTIES
    LABELS "cli;smoke")

add_test(
    NAME install_layout
    COMMAND
        ${CMAKE_COMMAND}
        "-DBUILD_DIR=${CMAKE_BINARY_DIR}"
        "-DOUT_DIR=${LPG2_CTEST_OUT_ROOT}/install_layout"
        "-DGRAMMAR=${_minimal}"
        "-DGLR_GRAMMAR=${LPG2_FIXTURES_DIR}/glr_expr.g"
        "-DBINARY_NAME=${_lpg2_binary_name}"
        "-DEXE_SUFFIX=${CMAKE_EXECUTABLE_SUFFIX}"
        "-DTEST_CONFIG=$<CONFIG>"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/run_install_test.cmake")
set_tests_properties(install_layout PROPERTIES
    LABELS "install;integration"
    RUN_SERIAL TRUE)

# Cross-backend AST shape differential: same S-expr dump across available backends.
# Depends on nested/list e2e tests writing ${OUT_DIR}/ast.sexpr via LPG2_AST_DUMP_PATH.
# Only register when ≥2 prerequisite automatic_ast tests exist; otherwise bare CI
# (no runtimes) would still run these tests because DEPENDS on unknown names is
# ignored by ctest. run_ast_shape_diff.cmake also skips if dumps are missing.
set(_ast_nested_prereqs
    cpp_automatic_ast_nested
    java_automatic_ast_nested
    python_automatic_ast_nested
    go_automatic_ast_nested
    typescript_automatic_ast_nested
    csharp_automatic_ast_nested
    dart_automatic_ast_nested
    rust_automatic_ast_nested_behavior)
set(_ast_nested_deps "")
foreach(_t IN LISTS _ast_nested_prereqs)
    if(TEST ${_t})
        list(APPEND _ast_nested_deps ${_t})
    endif()
endforeach()
list(LENGTH _ast_nested_deps _ast_nested_deps_n)
if(_ast_nested_deps_n GREATER_EQUAL 2)
    string(CONCAT _ast_nested_dumps
        "${LPG2_CTEST_OUT_ROOT}/cpp_automatic_ast_nested/ast.sexpr|"
        "${LPG2_CTEST_OUT_ROOT}/java_automatic_ast_nested/ast.sexpr|"
        "${LPG2_CTEST_OUT_ROOT}/python_automatic_ast_nested/ast.sexpr|"
        "${LPG2_CTEST_OUT_ROOT}/go_automatic_ast_nested/ast.sexpr|"
        "${LPG2_CTEST_OUT_ROOT}/typescript_automatic_ast_nested/ast.sexpr|"
        "${LPG2_CTEST_OUT_ROOT}/csharp_automatic_ast_nested/ast.sexpr|"
        "${LPG2_CTEST_OUT_ROOT}/dart_automatic_ast_nested/ast.sexpr|"
        "${LPG2_CTEST_OUT_ROOT}/rust_automatic_ast_nested_behavior/ast.sexpr")
    add_test(
        NAME ast_shape_diff_nested
        COMMAND
            ${CMAKE_COMMAND}
            "-DEXPECTED_DUMP=${CMAKE_CURRENT_SOURCE_DIR}/expected/ast_nested.sexpr"
            "-DDUMP_FILES=${_ast_nested_dumps}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/run_ast_shape_diff.cmake")
    set_tests_properties(ast_shape_diff_nested PROPERTIES
        LABELS "ast;diff;integration"
        DEPENDS "${_ast_nested_deps}")
else()
    message(STATUS
        "ast_shape_diff_nested skipped: need >= 2 automatic_ast nested backends "
        "(found ${_ast_nested_deps_n})")
endif()

set(_ast_list_prereqs
    cpp_automatic_ast_list
    java_automatic_ast_list
    rust_automatic_ast_list_behavior)
set(_ast_list_deps "")
foreach(_t IN LISTS _ast_list_prereqs)
    if(TEST ${_t})
        list(APPEND _ast_list_deps ${_t})
    endif()
endforeach()
list(LENGTH _ast_list_deps _ast_list_deps_n)
if(_ast_list_deps_n GREATER_EQUAL 2)
    string(CONCAT _ast_list_dumps
        "${LPG2_CTEST_OUT_ROOT}/cpp_automatic_ast_list/ast.sexpr|"
        "${LPG2_CTEST_OUT_ROOT}/java_automatic_ast_list/ast.sexpr|"
        "${LPG2_CTEST_OUT_ROOT}/rust_automatic_ast_list_behavior/ast.sexpr")
    add_test(
        NAME ast_shape_diff_list
        COMMAND
            ${CMAKE_COMMAND}
            "-DEXPECTED_DUMP=${CMAKE_CURRENT_SOURCE_DIR}/expected/ast_list.sexpr"
            "-DDUMP_FILES=${_ast_list_dumps}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/run_ast_shape_diff.cmake")
    set_tests_properties(ast_shape_diff_list PROPERTIES
        LABELS "ast;diff;integration"
        DEPENDS "${_ast_list_deps}")
else()
    message(STATUS
        "ast_shape_diff_list skipped: need >= 2 automatic_ast list backends "
        "(found ${_ast_list_deps_n})")
endif()
