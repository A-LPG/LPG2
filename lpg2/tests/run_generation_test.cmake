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
#
# Harnesses may write a cross-backend AST S-expr dump to $LPG2_AST_DUMP_PATH
# (set below to ${OUT_DIR}/ast.sexpr) for ast_shape_diff_* tests.

if(NOT DEFINED LPG2_EXE OR NOT DEFINED GRAMMAR OR NOT DEFINED LANG OR NOT DEFINED OUT_DIR)
    message(FATAL_ERROR "LPG2_EXE, GRAMMAR, LANG, and OUT_DIR are required")
endif()

# Make dump path available to parser harnesses (all languages).
set(ENV{LPG2_AST_DUMP_PATH} "${OUT_DIR}/ast.sexpr")

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

# Use a per-test cwd so temporary listing/tab stems (basename.l / .t) cannot
# collide when ctest launches multiple generators that share a grammar name.
# Keep cwd distinct from OUT_DIR itself so RelocateListingFile never tries to
# copy a listing onto itself (can fail with "File exists" when paths differ
# only by symlink, e.g. /tmp vs /private/tmp on macOS).
set(_gen_cwd "${OUT_DIR}/.lpg2-cwd")
file(MAKE_DIRECTORY "${_gen_cwd}")
execute_process(
    COMMAND ${_generator_command}
    WORKING_DIRECTORY "${_gen_cwd}"
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
        OR LANG STREQUAL "rt_cpp" OR LANG STREQUAL "cpp_legacy")
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
    if(NOT (LANG STREQUAL "cpp" OR LANG STREQUAL "rt_cpp" OR LANG STREQUAL "cpp2"
            OR LANG STREQUAL "cpp_legacy"))
        message(FATAL_ERROR
            "CHECK_CPP requires LANG=cpp, rt_cpp, cpp2, or cpp_legacy (got ${LANG})")
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

if(CHECK_JAVA)
    if(NOT LANG STREQUAL "java")
        message(FATAL_ERROR "CHECK_JAVA requires LANG=java")
    endif()
    if(NOT JAVA_PARSER)
        message(FATAL_ERROR "CHECK_JAVA currently requires JAVA_PARSER")
    endif()
    if(NOT DEFINED JAVA_JAVAC OR NOT DEFINED JAVA_JAVA)
        message(FATAL_ERROR "CHECK_JAVA requires JAVA_JAVAC and JAVA_JAVA")
    endif()
    if(NOT DEFINED JAVA_RUNTIME_DIR
            OR NOT EXISTS "${JAVA_RUNTIME_DIR}/src/lpg/runtime")
        message(FATAL_ERROR
            "JAVA_PARSER requires JAVA_RUNTIME_DIR with src/lpg/runtime "
            "(got '${JAVA_RUNTIME_DIR}')")
    endif()
    if(NOT DEFINED JAVA_HARNESS OR NOT EXISTS "${JAVA_HARNESS}")
        message(FATAL_ERROR "JAVA_PARSER requires JAVA_HARNESS")
    endif()

    set(_java_action "${OUT_DIR}/${EXPECT_PREFIX}.java")
    set(_java_prs "${OUT_DIR}/${EXPECT_PREFIX}prs.java")
    set(_java_sym "${OUT_DIR}/${EXPECT_PREFIX}sym.java")
    foreach(_f IN ITEMS "${_java_action}" "${_java_prs}" "${_java_sym}")
        if(NOT EXISTS "${_f}")
            message(FATAL_ERROR
                "Missing generated Java parser file: ${_f}\n"
                "Directory contents of ${OUT_DIR}:\n${_listing}")
        endif()
    endforeach()

    set(_java_project "${OUT_DIR}/java_check")
    set(_java_classes "${_java_project}/classes")
    file(REMOVE_RECURSE "${_java_project}")
    file(MAKE_DIRECTORY "${_java_classes}/lpg/runtime")

    set(_java_harness_out "${_java_project}/JavaParserHarness.java")
    configure_file("${JAVA_HARNESS}" "${_java_harness_out}" @ONLY)

    file(GLOB_RECURSE _java_runtime_sources
        "${JAVA_RUNTIME_DIR}/src/*.java")
    if(NOT _java_runtime_sources)
        message(FATAL_ERROR
            "No Java runtime sources under ${JAVA_RUNTIME_DIR}/src")
    endif()
    execute_process(
        COMMAND "${JAVA_JAVAC}" -encoding UTF-8 -d "${_java_classes}"
                ${_java_runtime_sources}
        RESULT_VARIABLE _javac_rt_rc
        OUTPUT_VARIABLE _javac_rt_out
        ERROR_VARIABLE _javac_rt_err)
    if(NOT _javac_rt_rc EQUAL 0)
        message(FATAL_ERROR
            "Failed to compile lpg-runtime (exit ${_javac_rt_rc})\n"
            "stdout:\n${_javac_rt_out}\nstderr:\n${_javac_rt_err}")
    endif()
    if(EXISTS "${JAVA_RUNTIME_DIR}/src/lpg/runtime/messages.properties")
        file(COPY "${JAVA_RUNTIME_DIR}/src/lpg/runtime/messages.properties"
            DESTINATION "${_java_classes}/lpg/runtime")
    endif()

    execute_process(
        COMMAND "${JAVA_JAVAC}" -encoding UTF-8 -cp "${_java_classes}"
                -d "${_java_classes}"
                "${_java_action}" "${_java_prs}" "${_java_sym}"
                "${_java_harness_out}"
        RESULT_VARIABLE _javac_gen_rc
        OUTPUT_VARIABLE _javac_gen_out
        ERROR_VARIABLE _javac_gen_err)
    if(NOT _javac_gen_rc EQUAL 0)
        message(FATAL_ERROR
            "Failed to compile generated Java parser (exit ${_javac_gen_rc})\n"
            "stdout:\n${_javac_gen_out}\nstderr:\n${_javac_gen_err}")
    endif()

    execute_process(
        COMMAND "${JAVA_JAVA}" -cp "${_java_classes}" JavaParserHarness
        RESULT_VARIABLE _java_run_rc
        OUTPUT_VARIABLE _java_run_out
        ERROR_VARIABLE _java_run_err)
    if(NOT _java_run_rc EQUAL 0)
        message(FATAL_ERROR
            "Generated Java parser harness failed (exit ${_java_run_rc})\n"
            "stdout:\n${_java_run_out}\nstderr:\n${_java_run_err}")
    endif()
endif()

if(CHECK_PYTHON)
    if(NOT LANG STREQUAL "python3")
        message(FATAL_ERROR "CHECK_PYTHON requires LANG=python3")
    endif()
    if(NOT PYTHON_PARSER)
        message(FATAL_ERROR "CHECK_PYTHON currently requires PYTHON_PARSER")
    endif()
    if(NOT DEFINED PYTHON_EXECUTABLE)
        message(FATAL_ERROR "CHECK_PYTHON requires PYTHON_EXECUTABLE")
    endif()
    if(NOT DEFINED PYTHON_RUNTIME_DIR
            OR NOT EXISTS "${PYTHON_RUNTIME_DIR}/lpg2")
        message(FATAL_ERROR
            "PYTHON_PARSER requires PYTHON_RUNTIME_DIR with lpg2 package "
            "(got '${PYTHON_RUNTIME_DIR}')")
    endif()
    if(NOT DEFINED PYTHON_HARNESS OR NOT EXISTS "${PYTHON_HARNESS}")
        message(FATAL_ERROR "PYTHON_PARSER requires PYTHON_HARNESS")
    endif()

    set(_py_action "${OUT_DIR}/${EXPECT_PREFIX}.py")
    set(_py_prs "${OUT_DIR}/${EXPECT_PREFIX}prs.py")
    set(_py_sym "${OUT_DIR}/${EXPECT_PREFIX}sym.py")
    foreach(_f IN ITEMS "${_py_action}" "${_py_prs}" "${_py_sym}")
        if(NOT EXISTS "${_f}")
            message(FATAL_ERROR
                "Missing generated Python parser file: ${_f}\n"
                "Directory contents of ${OUT_DIR}:\n${_listing}")
        endif()
    endforeach()

    set(_py_project "${OUT_DIR}/python_check")
    file(REMOVE_RECURSE "${_py_project}")
    file(MAKE_DIRECTORY "${_py_project}")
    set(_py_harness_out "${_py_project}/python_parser_harness.py")
    configure_file("${PYTHON_HARNESS}" "${_py_harness_out}" @ONLY)

    # PYTHONPATH: runtime package root + generated modules directory.
    if(WIN32)
        set(_py_path "${PYTHON_RUNTIME_DIR};${OUT_DIR}")
    else()
        set(_py_path "${PYTHON_RUNTIME_DIR}:${OUT_DIR}")
    endif()
    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E env "PYTHONPATH=${_py_path}"
                "${PYTHON_EXECUTABLE}" "${_py_harness_out}"
        RESULT_VARIABLE _py_run_rc
        OUTPUT_VARIABLE _py_run_out
        ERROR_VARIABLE _py_run_err)
    if(NOT _py_run_rc EQUAL 0)
        message(FATAL_ERROR
            "Generated Python parser harness failed (exit ${_py_run_rc})\n"
            "stdout:\n${_py_run_out}\nstderr:\n${_py_run_err}")
    endif()
endif()

if(CHECK_GO)
    if(NOT LANG STREQUAL "go")
        message(FATAL_ERROR "CHECK_GO requires LANG=go")
    endif()
    if(NOT GO_PARSER)
        message(FATAL_ERROR "CHECK_GO currently requires GO_PARSER")
    endif()
    if(NOT DEFINED GO_EXECUTABLE)
        message(FATAL_ERROR "CHECK_GO requires GO_EXECUTABLE")
    endif()
    if(NOT DEFINED GO_RUNTIME_DIR OR NOT EXISTS "${GO_RUNTIME_DIR}/go.mod")
        message(FATAL_ERROR
            "GO_PARSER requires GO_RUNTIME_DIR with go.mod "
            "(got '${GO_RUNTIME_DIR}')")
    endif()
    if(NOT DEFINED GO_HARNESS OR NOT EXISTS "${GO_HARNESS}")
        message(FATAL_ERROR "GO_PARSER requires GO_HARNESS")
    endif()

    set(_go_action "${OUT_DIR}/${EXPECT_PREFIX}.go")
    set(_go_prs "${OUT_DIR}/${EXPECT_PREFIX}prs.go")
    set(_go_sym "${OUT_DIR}/${EXPECT_PREFIX}sym.go")
    foreach(_f IN ITEMS "${_go_action}" "${_go_prs}" "${_go_sym}")
        if(NOT EXISTS "${_f}")
            message(FATAL_ERROR
                "Missing generated Go parser file: ${_f}\n"
                "Directory contents of ${OUT_DIR}:\n${_listing}")
        endif()
    endforeach()

    set(_go_project "${OUT_DIR}/go_check")
    file(REMOVE_RECURSE "${_go_project}")
    file(MAKE_DIRECTORY "${_go_project}")

    # Generated Go sources omit a package clause; wrap them as package main.
    foreach(_src IN ITEMS "${_go_action}" "${_go_prs}" "${_go_sym}")
        get_filename_component(_name "${_src}" NAME)
        file(READ "${_src}" _body)
        file(WRITE "${_go_project}/${_name}" "package main\n${_body}")
    endforeach()

    set(_go_harness_out "${_go_project}/main.go")
    configure_file("${GO_HARNESS}" "${_go_harness_out}" @ONLY)

    file(TO_CMAKE_PATH "${GO_RUNTIME_DIR}" _go_runtime_path)
    file(WRITE "${_go_project}/go.mod"
        "module lpg2_go_parser_check\n\n"
        "go 1.21\n\n"
        "require github.com/A-LPG/LPG-go-runtime v0.0.0\n\n"
        "replace github.com/A-LPG/LPG-go-runtime => ${_go_runtime_path}\n")

    execute_process(
        COMMAND "${GO_EXECUTABLE}" mod tidy
        WORKING_DIRECTORY "${_go_project}"
        RESULT_VARIABLE _go_tidy_rc
        OUTPUT_VARIABLE _go_tidy_out
        ERROR_VARIABLE _go_tidy_err)
    if(NOT _go_tidy_rc EQUAL 0)
        message(FATAL_ERROR
            "go mod tidy failed (exit ${_go_tidy_rc})\n"
            "stdout:\n${_go_tidy_out}\nstderr:\n${_go_tidy_err}")
    endif()

    execute_process(
        COMMAND "${GO_EXECUTABLE}" run .
        WORKING_DIRECTORY "${_go_project}"
        RESULT_VARIABLE _go_run_rc
        OUTPUT_VARIABLE _go_run_out
        ERROR_VARIABLE _go_run_err)
    if(NOT _go_run_rc EQUAL 0)
        message(FATAL_ERROR
            "Generated Go parser harness failed (exit ${_go_run_rc})\n"
            "stdout:\n${_go_run_out}\nstderr:\n${_go_run_err}")
    endif()
endif()

if(CHECK_TYPESCRIPT)
    if(NOT LANG STREQUAL "typescript")
        message(FATAL_ERROR "CHECK_TYPESCRIPT requires LANG=typescript")
    endif()
    if(NOT TYPESCRIPT_PARSER)
        message(FATAL_ERROR
            "CHECK_TYPESCRIPT currently requires TYPESCRIPT_PARSER")
    endif()
    if(NOT DEFINED NODE_EXECUTABLE OR NOT DEFINED NPM_EXECUTABLE)
        message(FATAL_ERROR
            "CHECK_TYPESCRIPT requires NODE_EXECUTABLE and NPM_EXECUTABLE")
    endif()
    if(NOT DEFINED TYPESCRIPT_RUNTIME_DIR
            OR NOT EXISTS "${TYPESCRIPT_RUNTIME_DIR}/package.json")
        message(FATAL_ERROR
            "TYPESCRIPT_PARSER requires TYPESCRIPT_RUNTIME_DIR with package.json "
            "(got '${TYPESCRIPT_RUNTIME_DIR}')")
    endif()
    if(NOT DEFINED TYPESCRIPT_HARNESS OR NOT EXISTS "${TYPESCRIPT_HARNESS}")
        message(FATAL_ERROR "TYPESCRIPT_PARSER requires TYPESCRIPT_HARNESS")
    endif()

    set(_ts_action "${OUT_DIR}/${EXPECT_PREFIX}.ts")
    set(_ts_prs "${OUT_DIR}/${EXPECT_PREFIX}prs.ts")
    set(_ts_sym "${OUT_DIR}/${EXPECT_PREFIX}sym.ts")
    foreach(_f IN ITEMS "${_ts_action}" "${_ts_prs}" "${_ts_sym}")
        if(NOT EXISTS "${_f}")
            message(FATAL_ERROR
                "Missing generated TypeScript parser file: ${_f}\n"
                "Directory contents of ${OUT_DIR}:\n${_listing}")
        endif()
    endforeach()

    set(_ts_project "${OUT_DIR}/typescript_check")
    file(REMOVE_RECURSE "${_ts_project}")
    file(MAKE_DIRECTORY "${_ts_project}")

    foreach(_src IN ITEMS "${_ts_action}" "${_ts_prs}" "${_ts_sym}")
        get_filename_component(_name "${_src}" NAME)
        file(READ "${_src}" _body)
        # Legacy templates escaped "./" as ".\/"; normalize for modern TS.
        string(REPLACE ".\\/" "./" _body "${_body}")
        file(WRITE "${_ts_project}/${_name}" "${_body}")
    endforeach()

    set(_ts_harness_out "${_ts_project}/harness.ts")
    configure_file("${TYPESCRIPT_HARNESS}" "${_ts_harness_out}" @ONLY)

    file(TO_CMAKE_PATH "${TYPESCRIPT_RUNTIME_DIR}" _ts_runtime_path)
    file(WRITE "${_ts_project}/package.json"
        "{\n"
        "  \"name\": \"lpg2-ts-parser-check\",\n"
        "  \"private\": true,\n"
        "  \"dependencies\": {\n"
        "    \"lpg2ts\": \"file:${_ts_runtime_path}\",\n"
        "    \"typescript\": \"^4.9.5\",\n"
        "    \"@types/node\": \"^14.17.6\"\n"
        "  }\n"
        "}\n")
    file(WRITE "${_ts_project}/tsconfig.json"
        "{\n"
        "  \"compilerOptions\": {\n"
        "    \"target\": \"ES2019\",\n"
        "    \"module\": \"commonjs\",\n"
        "    \"strict\": false,\n"
        "    \"esModuleInterop\": true,\n"
        "    \"skipLibCheck\": true,\n"
        "    \"outDir\": \"dist\",\n"
        "    \"rootDir\": \".\"\n"
        "  },\n"
        "  \"include\": [\"*.ts\"]\n"
        "}\n")

    execute_process(
        COMMAND "${NPM_EXECUTABLE}" install --silent
        WORKING_DIRECTORY "${_ts_project}"
        RESULT_VARIABLE _npm_rc
        OUTPUT_VARIABLE _npm_out
        ERROR_VARIABLE _npm_err)
    if(NOT _npm_rc EQUAL 0)
        message(FATAL_ERROR
            "npm install failed (exit ${_npm_rc})\n"
            "stdout:\n${_npm_out}\nstderr:\n${_npm_err}")
    endif()

    # Ensure the file: linked runtime has a compiled dist/ (incl. ExpectedTokens).
    # On a fresh checkout the runtime may lack node_modules/typescript.
    if(NOT EXISTS "${TYPESCRIPT_RUNTIME_DIR}/dist/index.js"
            OR NOT EXISTS "${TYPESCRIPT_RUNTIME_DIR}/dist/ExpectedTokens.js")
        if(NOT EXISTS "${TYPESCRIPT_RUNTIME_DIR}/node_modules/typescript")
            execute_process(
                COMMAND "${NPM_EXECUTABLE}" install --silent
                WORKING_DIRECTORY "${TYPESCRIPT_RUNTIME_DIR}"
                RESULT_VARIABLE _ts_rt_install_rc
                OUTPUT_VARIABLE _ts_rt_install_out
                ERROR_VARIABLE _ts_rt_install_err)
            if(NOT _ts_rt_install_rc EQUAL 0)
                message(FATAL_ERROR
                    "npm install for lpg2ts runtime failed (exit ${_ts_rt_install_rc})\n"
                    "stdout:\n${_ts_rt_install_out}\nstderr:\n${_ts_rt_install_err}")
            endif()
        endif()
        execute_process(
            COMMAND "${NPM_EXECUTABLE}" run build
            WORKING_DIRECTORY "${TYPESCRIPT_RUNTIME_DIR}"
            RESULT_VARIABLE _ts_rt_build_rc
            OUTPUT_VARIABLE _ts_rt_build_out
            ERROR_VARIABLE _ts_rt_build_err)
        if(NOT _ts_rt_build_rc EQUAL 0)
            message(FATAL_ERROR
                "Failed to build lpg2ts runtime (exit ${_ts_rt_build_rc})\n"
                "stdout:\n${_ts_rt_build_out}\nstderr:\n${_ts_rt_build_err}")
        endif()
    endif()

    execute_process(
        COMMAND "${NPM_EXECUTABLE}" exec -- tsc -p .
        WORKING_DIRECTORY "${_ts_project}"
        RESULT_VARIABLE _tsc_rc
        OUTPUT_VARIABLE _tsc_out
        ERROR_VARIABLE _tsc_err)
    if(NOT _tsc_rc EQUAL 0)
        message(FATAL_ERROR
            "tsc failed (exit ${_tsc_rc})\n"
            "stdout:\n${_tsc_out}\nstderr:\n${_tsc_err}")
    endif()

    execute_process(
        COMMAND "${NODE_EXECUTABLE}" "${_ts_project}/dist/harness.js"
        RESULT_VARIABLE _node_rc
        OUTPUT_VARIABLE _node_out
        ERROR_VARIABLE _node_err)
    if(NOT _node_rc EQUAL 0)
        message(FATAL_ERROR
            "Generated TypeScript parser harness failed (exit ${_node_rc})\n"
            "stdout:\n${_node_out}\nstderr:\n${_node_err}")
    endif()
endif()

if(CHECK_CSHARP)
    if(NOT LANG STREQUAL "csharp")
        message(FATAL_ERROR "CHECK_CSHARP requires LANG=csharp")
    endif()
    if(NOT CSHARP_PARSER)
        message(FATAL_ERROR "CHECK_CSHARP currently requires CSHARP_PARSER")
    endif()
    if(NOT DEFINED DOTNET_EXECUTABLE)
        message(FATAL_ERROR "CHECK_CSHARP requires DOTNET_EXECUTABLE")
    endif()
    if(NOT DEFINED CSHARP_RUNTIME_DIR
            OR NOT EXISTS "${CSHARP_RUNTIME_DIR}/LPG2.Runtime.csproj")
        message(FATAL_ERROR
            "CSHARP_PARSER requires CSHARP_RUNTIME_DIR with LPG2.Runtime.csproj "
            "(got '${CSHARP_RUNTIME_DIR}')")
    endif()
    if(NOT DEFINED CSHARP_HARNESS OR NOT EXISTS "${CSHARP_HARNESS}")
        message(FATAL_ERROR "CSHARP_PARSER requires CSHARP_HARNESS")
    endif()

    set(_cs_action "${OUT_DIR}/${EXPECT_PREFIX}.cs")
    set(_cs_prs "${OUT_DIR}/${EXPECT_PREFIX}prs.cs")
    set(_cs_sym "${OUT_DIR}/${EXPECT_PREFIX}sym.cs")
    foreach(_f IN ITEMS "${_cs_action}" "${_cs_prs}" "${_cs_sym}")
        if(NOT EXISTS "${_f}")
            message(FATAL_ERROR
                "Missing generated C# parser file: ${_f}\n"
                "Directory contents of ${OUT_DIR}:\n${_listing}")
        endif()
    endforeach()

    set(_cs_project "${OUT_DIR}/csharp_check")
    file(REMOVE_RECURSE "${_cs_project}")
    file(MAKE_DIRECTORY "${_cs_project}")

    foreach(_src IN ITEMS "${_cs_action}" "${_cs_prs}" "${_cs_sym}")
        get_filename_component(_name "${_src}" NAME)
        file(COPY "${_src}" DESTINATION "${_cs_project}")
    endforeach()

    set(_cs_harness_out "${_cs_project}/Program.cs")
    configure_file("${CSHARP_HARNESS}" "${_cs_harness_out}" @ONLY)

    file(TO_CMAKE_PATH "${CSHARP_RUNTIME_DIR}/LPG2.Runtime.csproj" _cs_runtime_proj)
    file(WRITE "${_cs_project}/csharp_check.csproj"
        "<Project Sdk=\"Microsoft.NET.Sdk\">\n"
        "  <PropertyGroup>\n"
        "    <OutputType>Exe</OutputType>\n"
        "    <TargetFramework>net8.0</TargetFramework>\n"
        "    <RollForward>LatestMajor</RollForward>\n"
        "    <ImplicitUsings>disable</ImplicitUsings>\n"
        "    <Nullable>disable</Nullable>\n"
        "  </PropertyGroup>\n"
        "  <ItemGroup>\n"
        "    <ProjectReference Include=\"${_cs_runtime_proj}\" />\n"
        "  </ItemGroup>\n"
        "</Project>\n")

    execute_process(
        COMMAND "${DOTNET_EXECUTABLE}" run --project "${_cs_project}/csharp_check.csproj"
            --configuration Release
        WORKING_DIRECTORY "${_cs_project}"
        RESULT_VARIABLE _cs_run_rc
        OUTPUT_VARIABLE _cs_run_out
        ERROR_VARIABLE _cs_run_err)
    if(NOT _cs_run_rc EQUAL 0)
        message(FATAL_ERROR
            "Generated C# parser harness failed (exit ${_cs_run_rc})\n"
            "stdout:\n${_cs_run_out}\nstderr:\n${_cs_run_err}")
    endif()
endif()

if(CHECK_DART)
    if(NOT LANG STREQUAL "dart")
        message(FATAL_ERROR "CHECK_DART requires LANG=dart")
    endif()
    if(NOT DART_PARSER)
        message(FATAL_ERROR "CHECK_DART currently requires DART_PARSER")
    endif()
    if(NOT DEFINED DART_EXECUTABLE)
        message(FATAL_ERROR "CHECK_DART requires DART_EXECUTABLE")
    endif()
    if(NOT DEFINED DART_RUNTIME_DIR OR NOT EXISTS "${DART_RUNTIME_DIR}/pubspec.yaml")
        message(FATAL_ERROR
            "DART_PARSER requires DART_RUNTIME_DIR with pubspec.yaml "
            "(got '${DART_RUNTIME_DIR}')")
    endif()
    if(NOT DEFINED DART_HARNESS OR NOT EXISTS "${DART_HARNESS}")
        message(FATAL_ERROR "DART_PARSER requires DART_HARNESS")
    endif()

    set(_dart_action "${OUT_DIR}/${EXPECT_PREFIX}.dart")
    set(_dart_prs "${OUT_DIR}/${EXPECT_PREFIX}prs.dart")
    set(_dart_sym "${OUT_DIR}/${EXPECT_PREFIX}sym.dart")
    foreach(_f IN ITEMS "${_dart_action}" "${_dart_prs}" "${_dart_sym}")
        if(NOT EXISTS "${_f}")
            message(FATAL_ERROR
                "Missing generated Dart parser file: ${_f}\n"
                "Directory contents of ${OUT_DIR}:\n${_listing}")
        endif()
    endforeach()

    set(_dart_project "${OUT_DIR}/dart_check")
    file(REMOVE_RECURSE "${_dart_project}")
    file(MAKE_DIRECTORY "${_dart_project}/bin")

    foreach(_src IN ITEMS "${_dart_action}" "${_dart_prs}" "${_dart_sym}")
        get_filename_component(_name "${_src}" NAME)
        file(COPY "${_src}" DESTINATION "${_dart_project}/bin")
    endforeach()

    set(_dart_harness_out "${_dart_project}/bin/harness.dart")
    configure_file("${DART_HARNESS}" "${_dart_harness_out}" @ONLY)

    # Copy runtime and relax SDK upper bound so Dart 3 can resolve path deps.
    set(_dart_runtime_copy "${_dart_project}/lpg2_runtime")
    file(COPY "${DART_RUNTIME_DIR}/" DESTINATION "${_dart_runtime_copy}"
        PATTERN ".git" EXCLUDE
        PATTERN ".dart_tool" EXCLUDE
        PATTERN "example" EXCLUDE)
    file(READ "${_dart_runtime_copy}/pubspec.yaml" _dart_rt_pubspec)
    string(REGEX REPLACE
        "sdk:[^\n]*"
        "sdk: '>=2.12.0 <4.0.0'"
        _dart_rt_pubspec "${_dart_rt_pubspec}")
    file(WRITE "${_dart_runtime_copy}/pubspec.yaml" "${_dart_rt_pubspec}")

    file(TO_CMAKE_PATH "${_dart_runtime_copy}" _dart_runtime_path)
    file(WRITE "${_dart_project}/pubspec.yaml"
        "name: lpg2_dart_parser_check\n"
        "publish_to: none\n"
        "environment:\n"
        "  sdk: '>=2.12.0 <4.0.0'\n"
        "dependencies:\n"
        "  lpg2:\n"
        "    path: ${_dart_runtime_path}\n")

    execute_process(
        COMMAND "${DART_EXECUTABLE}" pub get
        WORKING_DIRECTORY "${_dart_project}"
        RESULT_VARIABLE _dart_pub_rc
        OUTPUT_VARIABLE _dart_pub_out
        ERROR_VARIABLE _dart_pub_err)
    if(NOT _dart_pub_rc EQUAL 0)
        message(FATAL_ERROR
            "dart pub get failed (exit ${_dart_pub_rc})\n"
            "stdout:\n${_dart_pub_out}\nstderr:\n${_dart_pub_err}")
    endif()

    execute_process(
        COMMAND "${DART_EXECUTABLE}" run bin/harness.dart
        WORKING_DIRECTORY "${_dart_project}"
        RESULT_VARIABLE _dart_run_rc
        OUTPUT_VARIABLE _dart_run_out
        ERROR_VARIABLE _dart_run_err)
    if(NOT _dart_run_rc EQUAL 0)
        message(FATAL_ERROR
            "Generated Dart parser harness failed (exit ${_dart_run_rc})\n"
            "stdout:\n${_dart_run_out}\nstderr:\n${_dart_run_err}")
    endif()
endif()

# Optional: lock GLR nextAst scaffolding shape across backends (generation smoke).
if(CHECK_GLR_NEXTAST)
    set(_glr_need "")
    set(_glr_forbid "")
    if(LANG STREQUAL "java" OR LANG STREQUAL "csharp")
        set(_glr_need "IAst nextAst;getNextAst();setNextAst(IAst;resetNextAst()")
        set(_glr_forbid "ASTNode nextAst;nextAst = 0")
    elseif(LANG STREQUAL "cpp")
        set(_glr_need "IAst* nextAst;getNextAst();setNextAst(IAst*;resetNextAst()")
        set(_glr_forbid "ASTNode* nextAst;nextAst = 0")
    elseif(LANG STREQUAL "go")
        # Go emits "SetNextAst(n  IAst)"; keep a stable substring.
        set(_glr_need "nextAst IAst;GetNextAst();SetNextAst(n;ResetNextAst()")
        set(_glr_forbid "nextAst *ASTNode;nextAst = 0")
    elseif(LANG STREQUAL "typescript")
        set(_glr_need "nextAst : IAst;getNextAst();setNextAst(n : IAst;resetNextAst()")
        set(_glr_forbid "nextAst : ASTNode;nextAst = 0")
    elseif(LANG STREQUAL "dart")
        set(_glr_need "IAst? nextAst;getNextAst();setNextAst(IAst;resetNextAst()")
        set(_glr_forbid "ASTNode? nextAst;nextAst = 0")
    elseif(LANG STREQUAL "python3" OR LANG STREQUAL "python2")
        set(_glr_need "self.nextAst = None;getNextAst;setNextAst;resetNextAst")
        set(_glr_forbid "self.nextAst = 0")
    elseif(LANG STREQUAL "rust")
        set(_glr_need "next_ast:;get_next_ast;set_next_ast;reset_next_ast;Rc<dyn IAst>")
        set(_glr_forbid "next_ast: Option<Rc<ASTNode>>")
    else()
        message(FATAL_ERROR "CHECK_GLR_NEXTAST: unsupported LANG=${LANG}")
    endif()

    file(GLOB _glr_sources
        "${OUT_DIR}/*"
        "${OUT_DIR}/*/*"
        "${OUT_DIR}/*/*/*")
    set(_glr_blob "")
    foreach(_src IN LISTS _glr_sources)
        if(IS_DIRECTORY "${_src}")
            continue()
        endif()
        file(READ "${_src}" _chunk)
        string(APPEND _glr_blob "${_chunk}\n")
    endforeach()

    foreach(_pat IN LISTS _glr_need)
        string(FIND "${_glr_blob}" "${_pat}" _pos)
        if(_pos EQUAL -1)
            message(FATAL_ERROR
                "CHECK_GLR_NEXTAST (${LANG}): missing required nextAst pattern:\n"
                "  ${_pat}\n"
                "Directory contents of ${OUT_DIR}:\n${_listing}")
        endif()
    endforeach()
    foreach(_pat IN LISTS _glr_forbid)
        if("${_pat}" STREQUAL "")
            continue()
        endif()
        string(FIND "${_glr_blob}" "${_pat}" _pos)
        if(NOT _pos EQUAL -1)
            message(FATAL_ERROR
                "CHECK_GLR_NEXTAST (${LANG}): forbidden nextAst pattern present:\n"
                "  ${_pat}")
        endif()
    endforeach()
endif()
