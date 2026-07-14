set(LPG2_TEST_RUNNER "${CMAKE_CURRENT_SOURCE_DIR}/run_generation_test.cmake")
set(LPG2_FIXTURES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/fixtures")
set(LPG2_CTEST_OUT_ROOT "${CMAKE_BINARY_DIR}/ctest")

add_executable(lpg2_container_tests container_tests.cpp)
target_include_directories(lpg2_container_tests PRIVATE
    "${CMAKE_SOURCE_DIR}/include")
target_compile_features(lpg2_container_tests PRIVATE cxx_std_17)
if(MSVC)
    target_compile_options(lpg2_container_tests PRIVATE /W4 /WX)
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
    target_compile_options(lpg2_output_transaction_tests PRIVATE /W4 /WX)
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
if(MSVC)
    target_compile_options(lpg2_resource_path_tests PRIVATE /W4 /WX)
else()
    target_compile_options(lpg2_resource_path_tests PRIVATE
        -Wall -Wextra -Werror)
endif()
add_test(NAME resource_discovery COMMAND lpg2_resource_path_tests)
set_tests_properties(resource_discovery PROPERTIES
    LABELS "install;unit")

# Rust integration is enabled automatically when cargo and the sibling runtime
# are available. CI sets LPG2_REQUIRE_RUST_TESTS=ON so it cannot be skipped.
find_program(CARGO_EXECUTABLE cargo)
set(LPG2_RUST_TEST_RUNTIME_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/rust-runtime-shim"
    CACHE PATH "Dependency-free Rust runtime ABI shim used by table tests")
set(LPG2_RUST_RUNTIME_DIR
    "${CMAKE_SOURCE_DIR}/../../LPG-rust-runtime/lpg2"
    CACHE PATH "Path to the LPG Rust runtime crate")
option(LPG2_REQUIRE_RUST_TESTS
    "Fail configure when Rust table compile/run tests cannot be enabled" OFF)
option(LPG2_REQUIRE_RUST_PARSER_TESTS
    "Fail configure when full Rust runtime parser tests cannot be enabled" OFF)

# lpg2_add_generation_test(<name> <grammar> <lang>
#   [LABELS ...] [EXPECT_FAIL] [EXPECT_NO_OUTPUT] [CHECK_CPP] [CHECK_RUST]
#   [CPP_PARSE] [RUST_PARSE] [RUST_PARSER]
#   [PREFIX <prefix>] [EXPECT_EXIT_CODE <code>] [EXPECT_MESSAGE <substring>]
#   [TEMPLATE <path>] [INCLUDE_DIR <path>] [RUNTIME_DIR <crate path>])
function(lpg2_add_generation_test NAME GRAMMAR LANG)
    set(options
        EXPECT_FAIL EXPECT_NO_OUTPUT CHECK_CPP CHECK_RUST
        CPP_PARSE RUST_PARSE RUST_PARSER)
    set(oneValueArgs
        PREFIX EXPECT_EXIT_CODE EXPECT_MESSAGE TEMPLATE INCLUDE_DIR RUNTIME_DIR)
    set(multiValueArgs LABELS)
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
    if(ARG_CHECK_CPP)
        list(APPEND _cmd
            "-DCHECK_CPP=TRUE"
            "-DGENERATOR=${CMAKE_GENERATOR}"
            "-DTEST_CONFIG=$<CONFIG>"
            "-DCMAKE_CTEST_COMMAND=${CMAKE_CTEST_COMMAND}")
        if(ARG_CPP_PARSE)
            list(APPEND _cmd
                "-DCPP_PARSE=TRUE"
                "-DCPP_HARNESS=${CMAKE_CURRENT_SOURCE_DIR}/harness/cpp_table_harness.cpp.in")
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
            "-DRUST_RUNTIME_DIR=${_rust_runtime_dir}")
        if(ARG_RUST_PARSE)
            list(APPEND _cmd
                "-DRUST_PARSE=TRUE"
                "-DRUST_HARNESS=${CMAKE_CURRENT_SOURCE_DIR}/harness/rust_table_harness.rs.in")
        endif()
        if(ARG_RUST_PARSER)
            list(APPEND _cmd
                "-DRUST_PARSER=TRUE"
                "-DRUST_PARSER_HARNESS=${CMAKE_CURRENT_SOURCE_DIR}/harness/rust_parser_harness.rs.in")
        endif()
    endif()
    list(APPEND _cmd -P "${LPG2_TEST_RUNNER}")

    add_test(NAME ${NAME} COMMAND ${_cmd})
    if(ARG_LABELS)
        set_tests_properties(${NAME} PROPERTIES LABELS "${ARG_LABELS}")
    endif()
endfunction()

# --- Tier A: fast multi-backend smoke (minimal.g) ---
set(_minimal "${LPG2_FIXTURES_DIR}/minimal.g")
foreach(_lang IN ITEMS cpp java go python3 csharp typescript dart rust)
    lpg2_add_generation_test(minimal_${_lang} "${_minimal}" ${_lang} LABELS smoke)
endforeach()

# Compile and run generated tables instead of only checking file existence.
lpg2_add_generation_test(generated_cpp_compile_run "${_minimal}" cpp
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
lpg2_add_generation_test(backtrack_cpp_compile_run "${_backtrack}" cpp
    CHECK_CPP CPP_PARSE LABELS compile integration backtracking parser)
if(_rust_tests_available)
    lpg2_add_generation_test(backtrack_rust_compile_run "${_backtrack}" rust
        CHECK_RUST
        RUNTIME_DIR "${LPG2_RUST_TEST_RUNTIME_DIR}"
        LABELS compile integration backtracking)
endif()

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

lpg2_add_generation_test(rust_automatic_ast_rejected
    "${LPG2_FIXTURES_DIR}/ast_nested.g" rust
    EXPECT_EXIT_CODE 12
    EXPECT_MESSAGE "Rust automatic AST generation is not supported"
    EXPECT_NO_OUTPUT
    TEMPLATE "${_rust_template_dir}/dtParserTemplateF.gi"
    INCLUDE_DIR "${_rust_include_dir}"
    LABELS feature negative rust)

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
add_test(
    NAME bootstrap_stage2
    COMMAND
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
        -P "${CMAKE_CURRENT_SOURCE_DIR}/run_bootstrap_test.cmake")
set_tests_properties(bootstrap_stage2 PROPERTIES
    LABELS "bootstrap;integration"
    RUN_SERIAL TRUE
    TIMEOUT 300)

# --- Tier C: feature / error-path regression ---
lpg2_add_generation_test(dropactions_import
    "${LPG2_FIXTURES_DIR}/import_dropactions.g" cpp
    PREFIX import_dropactions LABELS smoke feature)

lpg2_add_generation_test(invalid_lang_fails
    "${_minimal}" notalang
    EXPECT_EXIT_CODE 12
    EXPECT_MESSAGE "invalid value"
    EXPECT_NO_OUTPUT
    LABELS smoke feature)

lpg2_add_generation_test(invalid_grammar_fails
    "${LPG2_FIXTURES_DIR}/invalid.g" cpp
    EXPECT_EXIT_CODE 12
    EXPECT_MESSAGE "Error"
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
        "-DBINARY_NAME=${_lpg2_binary_name}"
        "-DEXE_SUFFIX=${CMAKE_EXECUTABLE_SUFFIX}"
        "-DTEST_CONFIG=$<CONFIG>"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/run_install_test.cmake")
set_tests_properties(install_layout PROPERTIES
    LABELS "install;integration"
    RUN_SERIAL TRUE)
