# Unified LPG2 generation test runner.
# Required variables: LPG2_EXE, GRAMMAR, LANG, OUT_DIR
# Optional:
#   EXPECT_PREFIX=<prefix>
#   EXPECT_FAIL=TRUE
#   EXPECT_EXIT_CODE=<code>
#   EXPECT_MESSAGE=<literal substring>
#   EXPECT_NO_OUTPUT=TRUE
#   EXTRA_ARGS=<space- or semicolon-separated CLI args>
#   CHECK_GOLDEN=TRUE, GOLDEN_DIR=<dir with expected *prs.* files>
#   CHECK_CPP=TRUE, CXX_COMPILER, CXX_COMPILER_ID, EXE_SUFFIX
#   CHECK_RUST=TRUE, CARGO_EXECUTABLE, RUST_RUNTIME_DIR

if(NOT DEFINED LPG2_EXE OR NOT DEFINED GRAMMAR OR NOT DEFINED LANG OR NOT DEFINED OUT_DIR)
    message(FATAL_ERROR "LPG2_EXE, GRAMMAR, LANG, and OUT_DIR are required")
endif()

if(NOT DEFINED EXPECT_PREFIX)
    get_filename_component(EXPECT_PREFIX "${GRAMMAR}" NAME_WE)
endif()

file(REMOVE_RECURSE "${OUT_DIR}")
file(MAKE_DIRECTORY "${OUT_DIR}")

set(_generator_command
    "${LPG2_EXE}"
    "-programming_language=${LANG}"
    -table
    -quiet
    "-out_directory=${OUT_DIR}")
if(DEFINED TEMPLATE AND NOT "${TEMPLATE}" STREQUAL "")
    list(APPEND _generator_command "-template=${TEMPLATE}")
endif()
if(DEFINED INCLUDE_DIR AND NOT "${INCLUDE_DIR}" STREQUAL "")
    list(APPEND _generator_command "-include-directory=${INCLUDE_DIR}")
endif()
if(DEFINED EXTRA_ARGS AND NOT "${EXTRA_ARGS}" STREQUAL "")
    string(REPLACE ";" " " _extra_args_normalized "${EXTRA_ARGS}")
    separate_arguments(_extra_args_list UNIX_COMMAND "${_extra_args_normalized}")
    list(APPEND _generator_command ${_extra_args_list})
endif()
list(APPEND _generator_command "${GRAMMAR}")

execute_process(
    COMMAND ${_generator_command}
    RESULT_VARIABLE _rc
    OUTPUT_VARIABLE _out
    ERROR_VARIABLE _err
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE
)

set(_diagnostics "${_out}\n${_err}")

if(DEFINED EXPECT_EXIT_CODE)
    if(NOT _rc EQUAL EXPECT_EXIT_CODE)
        message(FATAL_ERROR
            "Expected exit ${EXPECT_EXIT_CODE} for LANG=${LANG}, got ${_rc}\n"
            "stdout:\n${_out}\nstderr:\n${_err}")
    endif()
elseif(EXPECT_FAIL)
    if(_rc EQUAL 0)
        message(FATAL_ERROR
            "Expected non-zero exit for LANG=${LANG}, but got 0\n"
            "stdout:\n${_out}\nstderr:\n${_err}")
    endif()
endif()

if(DEFINED EXPECT_MESSAGE)
    string(FIND "${_diagnostics}" "${EXPECT_MESSAGE}" _message_pos)
    if(_message_pos EQUAL -1)
        message(FATAL_ERROR
            "Expected diagnostic substring not found: ${EXPECT_MESSAGE}\n"
            "stdout:\n${_out}\nstderr:\n${_err}")
    endif()
endif()

if(EXPECT_FAIL OR DEFINED EXPECT_EXIT_CODE)
    if(EXPECT_NO_OUTPUT)
        file(GLOB _unexpected_outputs
            "${OUT_DIR}/*prs.*"
            "${OUT_DIR}/*sym.*")
        if(_unexpected_outputs)
            message(FATAL_ERROR
                "Failure left generated table files behind: ${_unexpected_outputs}")
        endif()
    endif()
    return()
endif()

if(NOT _rc EQUAL 0)
    message(FATAL_ERROR
        "lpg2 failed (exit ${_rc}) LANG=${LANG} GRAMMAR=${GRAMMAR}\n"
        "stdout:\n${_out}\nstderr:\n${_err}")
endif()

# Map programming_language -> table file extension (see option.cpp help_get_file).
if(LANG STREQUAL "cpp" OR LANG STREQUAL "c" OR LANG STREQUAL "cpp2"
        OR LANG STREQUAL "rt_cpp")
    set(_ext "h")
elseif(LANG STREQUAL "java")
    set(_ext "java")
elseif(LANG STREQUAL "go")
    set(_ext "go")
elseif(LANG STREQUAL "python2" OR LANG STREQUAL "python3")
    set(_ext "py")
elseif(LANG STREQUAL "csharp")
    set(_ext "cs")
elseif(LANG STREQUAL "typescript")
    set(_ext "ts")
elseif(LANG STREQUAL "dart")
    set(_ext "dart")
elseif(LANG STREQUAL "rust")
    set(_ext "rs")
else()
    message(FATAL_ERROR "Unknown LANG=${LANG} for output extension mapping")
endif()

set(_prs "${OUT_DIR}/${EXPECT_PREFIX}prs.${_ext}")
set(_sym "${OUT_DIR}/${EXPECT_PREFIX}sym.${_ext}")

file(GLOB _listing RELATIVE "${OUT_DIR}" "${OUT_DIR}/*")
foreach(_f IN ITEMS "${_prs}" "${_sym}")
    if(NOT EXISTS "${_f}")
        message(FATAL_ERROR
            "Missing expected output file: ${_f}\n"
            "Directory contents of ${OUT_DIR}:\n${_listing}")
    endif()
    file(SIZE "${_f}" _sz)
    if(_sz EQUAL 0)
        message(FATAL_ERROR "Expected non-empty output file: ${_f}")
    endif()
endforeach()

if(CHECK_GOLDEN)
    if(NOT DEFINED GOLDEN_DIR OR "${GOLDEN_DIR}" STREQUAL "")
        message(FATAL_ERROR "CHECK_GOLDEN requires GOLDEN_DIR")
    endif()
    if(NOT IS_DIRECTORY "${GOLDEN_DIR}")
        message(FATAL_ERROR "GOLDEN_DIR does not exist: ${GOLDEN_DIR}")
    endif()
    file(GLOB _generated_prs "${OUT_DIR}/${EXPECT_PREFIX}prs.*")
    if(NOT _generated_prs)
        message(FATAL_ERROR
            "CHECK_GOLDEN found no ${EXPECT_PREFIX}prs.* under ${OUT_DIR}")
    endif()
    foreach(_gen IN LISTS _generated_prs)
        get_filename_component(_name "${_gen}" NAME)
        set(_golden "${GOLDEN_DIR}/${_name}")
        if(NOT EXISTS "${_golden}")
            message(FATAL_ERROR "Missing golden file: ${_golden}")
        endif()
        # Normalize CRLF so Windows checkouts match LF goldens / generators.
        file(READ "${_gen}" _gen_text)
        file(READ "${_golden}" _golden_text)
        string(REPLACE "\r\n" "\n" _gen_text "${_gen_text}")
        string(REPLACE "\r\n" "\n" _golden_text "${_golden_text}")
        string(REPLACE "\r" "\n" _gen_text "${_gen_text}")
        string(REPLACE "\r" "\n" _golden_text "${_golden_text}")
        if(NOT _gen_text STREQUAL _golden_text)
            file(SHA256 "${_gen}" _gen_hash)
            file(SHA256 "${_golden}" _golden_hash)
            message(FATAL_ERROR
                "Golden content mismatch for ${_name}\n"
                "  generated SHA256: ${_gen_hash}\n"
                "  golden SHA256:    ${_golden_hash}\n"
                "  generated path: ${_gen}\n"
                "  golden path:    ${_golden}")
        endif()
    endforeach()
endif()

if(CHECK_CPP)
    if(NOT (LANG STREQUAL "cpp" OR LANG STREQUAL "rt_cpp" OR LANG STREQUAL "cpp2"))
        message(FATAL_ERROR
            "CHECK_CPP requires LANG=cpp, rt_cpp, or cpp2 (got ${LANG})")
    endif()

    set(_cpp_project "${OUT_DIR}/cpp_check")
    set(_cpp_build "${_cpp_project}/build")
    file(MAKE_DIRECTORY "${_cpp_project}")
    set(_cpp_main "${_cpp_project}/generated_parser_check.cpp")
    file(TO_CMAKE_PATH "${OUT_DIR}" _cpp_include_path)

    if(CPP_PARSER)
        set(_cpp_action "${OUT_DIR}/${EXPECT_PREFIX}.h")
        set(_cpp_action_impl "${OUT_DIR}/${EXPECT_PREFIX}.cpp")
        set(_cpp_prs_h "${OUT_DIR}/${EXPECT_PREFIX}prs.h")
        set(_cpp_sym_h "${OUT_DIR}/${EXPECT_PREFIX}sym.h")
        foreach(_f IN ITEMS
                "${_cpp_action}" "${_cpp_action_impl}"
                "${_cpp_prs_h}" "${_cpp_sym_h}")
            if(NOT EXISTS "${_f}")
                message(FATAL_ERROR
                    "Missing generated C++ parser file: ${_f}\n"
                    "Directory contents of ${OUT_DIR}:\n${_listing}")
            endif()
        endforeach()
        if(NOT DEFINED CPP_HARNESS OR NOT EXISTS "${CPP_HARNESS}")
            message(FATAL_ERROR "CPP_PARSER requires CPP_HARNESS")
        endif()
        if(NOT DEFINED CPP_RUNTIME_DIR
                OR NOT EXISTS "${CPP_RUNTIME_DIR}/CMakeLists.txt")
            message(FATAL_ERROR
                "CPP_PARSER requires CPP_RUNTIME_DIR with CMakeLists.txt "
                "(got '${CPP_RUNTIME_DIR}')")
        endif()
        # Nested project uses generated_parser_check.cpp as the harness name.
        configure_file("${CPP_HARNESS}" "${_cpp_main}" @ONLY)
        file(TO_CMAKE_PATH "${CPP_RUNTIME_DIR}" _cpp_runtime_path)
        file(TO_CMAKE_PATH "${_cpp_action_impl}" _cpp_action_impl_path)
        file(WRITE "${_cpp_project}/CMakeLists.txt"
            "cmake_minimum_required(VERSION 3.16)\n"
            "project(lpg2_generated_parser_check LANGUAGES CXX C)\n"
            "enable_testing()\n"
            "set(CPPLPG2_BUILD_EXAMPLES OFF CACHE BOOL \"\" FORCE)\n"
            "set(CPPLPG2_INSTALL OFF CACHE BOOL \"\" FORCE)\n"
            "if(NOT MSVC)\n"
            "  add_compile_options(-w)\n"
            "endif()\n"
            "add_subdirectory(\"${_cpp_runtime_path}\" cpplpg2_build "
                "EXCLUDE_FROM_ALL)\n"
            "add_executable(generated_parser_check\n"
            "    generated_parser_check.cpp\n"
            "    \"${_cpp_action_impl_path}\")\n"
            "target_include_directories(generated_parser_check PRIVATE "
                "\"${_cpp_include_path}\")\n"
            "target_link_libraries(generated_parser_check PRIVATE cpplpg2)\n"
            "target_compile_features(generated_parser_check PRIVATE "
                "cxx_std_17)\n"
            "add_test(NAME run_generated_parser "
                "COMMAND generated_parser_check)\n")
    else()
        set(_cpp_impl "${OUT_DIR}/${EXPECT_PREFIX}prs.cpp")
        if(NOT EXISTS "${_cpp_impl}")
            message(FATAL_ERROR
                "Missing generated C++ implementation: ${_cpp_impl}")
        endif()
        set(_cpp_main "${_cpp_project}/generated_table_check.cpp")
        if(CPP_PARSE)
            if(NOT DEFINED CPP_HARNESS OR NOT EXISTS "${CPP_HARNESS}")
                message(FATAL_ERROR "CPP_PARSE requires CPP_HARNESS")
            endif()
            configure_file("${CPP_HARNESS}" "${_cpp_main}" @ONLY)
        else()
            file(WRITE "${_cpp_main}"
                "#include \"${EXPECT_PREFIX}prs.h\"\n"
                "int main() {\n"
                "    if (IS_VALID_FOR_PARSER != 1) return 1;\n"
                "    const int action = ${EXPECT_PREFIX}prs::t_action("
                    "${EXPECT_PREFIX}prs::START_STATE, a);\n"
                "    return action == ${EXPECT_PREFIX}prs::ERROR_ACTION "
                    "? 2 : 0;\n"
                "}\n")
        endif()

        file(TO_CMAKE_PATH "${_cpp_impl}" _cpp_impl_path)
        file(WRITE "${_cpp_project}/CMakeLists.txt"
            "cmake_minimum_required(VERSION 3.16)\n"
            "project(lpg2_generated_table_check LANGUAGES CXX)\n"
            "enable_testing()\n"
            "add_executable(generated_table_check\n"
            "    generated_table_check.cpp\n"
            "    \"${_cpp_impl_path}\")\n"
            "target_include_directories(generated_table_check PRIVATE "
                "\"${_cpp_include_path}\")\n"
            "target_compile_features(generated_table_check PRIVATE "
                "cxx_std_17)\n"
            "add_test(NAME run_generated_table "
                "COMMAND generated_table_check)\n")
    endif()

    set(_configure_cmd "${CMAKE_COMMAND}" -S "${_cpp_project}" -B "${_cpp_build}"
        -DCMAKE_BUILD_TYPE=Release)
    if(DEFINED CMAKE_CXX_COMPILER AND NOT "${CMAKE_CXX_COMPILER}" STREQUAL "")
        list(APPEND _configure_cmd "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}")
    endif()
    if(DEFINED CMAKE_C_COMPILER AND NOT "${CMAKE_C_COMPILER}" STREQUAL "")
        list(APPEND _configure_cmd "-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}")
    endif()
    if(DEFINED CMAKE_MAKE_PROGRAM AND NOT "${CMAKE_MAKE_PROGRAM}" STREQUAL "")
        list(APPEND _configure_cmd "-DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}")
    endif()
    if(DEFINED GENERATOR AND NOT "${GENERATOR}" STREQUAL "")
        list(APPEND _configure_cmd -G "${GENERATOR}")
    endif()
    if(DEFINED GENERATOR_PLATFORM AND NOT "${GENERATOR_PLATFORM}" STREQUAL "")
        list(APPEND _configure_cmd -A "${GENERATOR_PLATFORM}")
    endif()
    if(DEFINED GENERATOR_TOOLSET AND NOT "${GENERATOR_TOOLSET}" STREQUAL "")
        list(APPEND _configure_cmd -T "${GENERATOR_TOOLSET}")
    endif()
    if(CPP_PARSER AND DEFINED CPP_RUNTIME_DIR)
        list(APPEND _configure_cmd "-DCPP_RUNTIME_DIR=${CPP_RUNTIME_DIR}")
    endif()
    execute_process(
        COMMAND ${_configure_cmd}
        RESULT_VARIABLE _configure_rc
        OUTPUT_VARIABLE _configure_out
        ERROR_VARIABLE _configure_err)
    if(NOT _configure_rc EQUAL 0)
        message(FATAL_ERROR
            "Failed to configure generated C++ check "
            "(exit ${_configure_rc})\n"
            "stdout:\n${_configure_out}\nstderr:\n${_configure_err}")
    endif()

    if(NOT DEFINED TEST_CONFIG OR "${TEST_CONFIG}" STREQUAL "")
        set(TEST_CONFIG Release)
    endif()
    execute_process(
        COMMAND "${CMAKE_COMMAND}" --build "${_cpp_build}"
                --config "${TEST_CONFIG}"
        RESULT_VARIABLE _compile_rc
        OUTPUT_VARIABLE _compile_out
        ERROR_VARIABLE _compile_err)
    if(NOT _compile_rc EQUAL 0)
        set(_compile_diagnostics "${_compile_out}\n${_compile_err}")
        string(REGEX MATCHALL
            "[^\n]*(error:|undefined reference|FAILED:|ninja: build stopped)[^\n]*"
            _compile_error_lines "${_compile_diagnostics}")
        list(JOIN _compile_error_lines "\n" _compile_error_summary)
        message(FATAL_ERROR
            "Generated C++ check failed to compile (exit ${_compile_rc})\n"
            "error summary:\n${_compile_error_summary}\n"
            "stdout:\n${_compile_out}\nstderr:\n${_compile_err}")
    endif()

    execute_process(
        COMMAND "${CMAKE_CTEST_COMMAND}" --test-dir "${_cpp_build}"
                -C "${TEST_CONFIG}" --output-on-failure
        RESULT_VARIABLE _run_rc
        OUTPUT_VARIABLE _run_out
        ERROR_VARIABLE _run_err)
    if(NOT _run_rc EQUAL 0)
        message(FATAL_ERROR
            "Generated C++ check failed (exit ${_run_rc})\n"
            "stdout:\n${_run_out}\nstderr:\n${_run_err}")
    endif()
endif()

if(CHECK_RUST)
    if(NOT LANG STREQUAL "rust")
        message(FATAL_ERROR "CHECK_RUST requires LANG=rust")
    endif()
    if(NOT DEFINED CARGO_EXECUTABLE OR NOT DEFINED RUST_RUNTIME_DIR)
        message(FATAL_ERROR
            "CHECK_RUST requires CARGO_EXECUTABLE and RUST_RUNTIME_DIR")
    endif()
    if(NOT EXISTS "${RUST_RUNTIME_DIR}/Cargo.toml")
        message(FATAL_ERROR
            "Rust runtime crate not found: ${RUST_RUNTIME_DIR}")
    endif()

    set(_rust_project "${OUT_DIR}/rust_check")
    set(_rust_src "${_rust_project}/src")
    file(MAKE_DIRECTORY "${_rust_src}")
    file(COPY "${_prs}" "${_sym}" DESTINATION "${_rust_src}")
    if(RUST_PARSER)
        set(_rust_action "${OUT_DIR}/${EXPECT_PREFIX}.rs")
        if(NOT EXISTS "${_rust_action}")
            message(FATAL_ERROR
                "Missing generated Rust parser implementation: ${_rust_action}")
        endif()
        file(COPY "${_rust_action}" DESTINATION "${_rust_src}")
    endif()
    file(TO_CMAKE_PATH "${RUST_RUNTIME_DIR}" _rust_runtime_path)

    # Unique crate name per OUT_DIR so a shared CARGO_TARGET_DIR cannot collide
    # when several generation tests build packages named the same way.
    get_filename_component(_rust_pkg_stem "${OUT_DIR}" NAME)
    string(REGEX REPLACE "[^A-Za-z0-9_-]" "_" _rust_pkg_stem "${_rust_pkg_stem}")
    if(_rust_pkg_stem STREQUAL "")
        set(_rust_pkg_stem "generated")
    endif()

    file(WRITE "${_rust_project}/Cargo.toml"
        "[package]\n"
        "name = \"lpg2-check-${_rust_pkg_stem}\"\n"
        "version = \"0.0.0\"\n"
        "edition = \"2021\"\n"
        "publish = false\n\n"
        "[dependencies]\n"
        "lpg2 = { path = \"${_rust_runtime_path}\" }\n")
    if(RUST_PARSER)
        if(NOT DEFINED RUST_PARSER_HARNESS
                OR NOT EXISTS "${RUST_PARSER_HARNESS}")
            message(FATAL_ERROR
                "RUST_PARSER requires RUST_PARSER_HARNESS")
        endif()
        configure_file(
            "${RUST_PARSER_HARNESS}" "${_rust_src}/lib.rs" @ONLY)
    elseif(RUST_PARSE)
        if(NOT DEFINED RUST_HARNESS OR NOT EXISTS "${RUST_HARNESS}")
            message(FATAL_ERROR "RUST_PARSE requires RUST_HARNESS")
        endif()
        configure_file("${RUST_HARNESS}" "${_rust_src}/lib.rs" @ONLY)
    else()
        file(WRITE "${_rust_src}/lib.rs"
            "#![allow(dead_code, non_camel_case_types, non_upper_case_globals)]\n"
            "include!(\"${EXPECT_PREFIX}sym.rs\");\n"
            "include!(\"${EXPECT_PREFIX}prs.rs\");\n\n"
            "#[cfg(test)]\n"
            "mod tests {\n"
            "    use super::*;\n"
            "    use lpg2::traits::ParseTable;\n\n"
            "    #[test]\n"
            "    fn generated_table_is_usable() {\n"
            "        let table = ${EXPECT_PREFIX}prs::new();\n"
            "        assert!(table.is_valid_for_parser());\n"
            "        assert!(table.get_num_states() > 0);\n"
            "        assert_ne!(table.t_action(table.get_start_state(), "
                "${EXPECT_PREFIX}sym::a), table.get_error_action());\n"
            "    }\n"
            "}\n")
    endif()

    if(DEFINED LPG2_CARGO_TARGET_DIR AND NOT "${LPG2_CARGO_TARGET_DIR}" STREQUAL "")
        set(_cargo_target_dir "${LPG2_CARGO_TARGET_DIR}")
    else()
        set(_cargo_target_dir "${_rust_project}/target")
    endif()
    file(MAKE_DIRECTORY "${_cargo_target_dir}")

    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E env
                "CARGO_TARGET_DIR=${_cargo_target_dir}"
                "${CARGO_EXECUTABLE}" test
                --manifest-path "${_rust_project}/Cargo.toml"
                --quiet
        RESULT_VARIABLE _cargo_rc
        OUTPUT_VARIABLE _cargo_out
        ERROR_VARIABLE _cargo_err)
    if(NOT _cargo_rc EQUAL 0)
        message(FATAL_ERROR
            "Generated Rust table failed cargo test (exit ${_cargo_rc})\n"
            "stdout:\n${_cargo_out}\nstderr:\n${_cargo_err}")
    endif()
endif()
