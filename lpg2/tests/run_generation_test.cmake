# Unified LPG2 generation test runner.
# Required variables: LPG2_EXE, GRAMMAR, LANG, OUT_DIR, EXPECT_PREFIX
# Optional: EXPECT_FAIL=TRUE  (expect non-zero exit; skip output checks)

if(NOT DEFINED LPG2_EXE OR NOT DEFINED GRAMMAR OR NOT DEFINED LANG OR NOT DEFINED OUT_DIR)
    message(FATAL_ERROR "LPG2_EXE, GRAMMAR, LANG, and OUT_DIR are required")
endif()

if(NOT DEFINED EXPECT_PREFIX)
    get_filename_component(EXPECT_PREFIX "${GRAMMAR}" NAME_WE)
endif()

file(REMOVE_RECURSE "${OUT_DIR}")
file(MAKE_DIRECTORY "${OUT_DIR}")

execute_process(
    COMMAND "${LPG2_EXE}"
            "-programming_language=${LANG}"
            -table
            -quiet
            "-out_directory=${OUT_DIR}"
            "${GRAMMAR}"
    RESULT_VARIABLE _rc
    OUTPUT_VARIABLE _out
    ERROR_VARIABLE _err
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE
)

if(EXPECT_FAIL)
    if(_rc EQUAL 0)
        message(FATAL_ERROR
            "Expected non-zero exit for LANG=${LANG}, but got 0\n"
            "stdout:\n${_out}\nstderr:\n${_err}")
    endif()
    return()
endif()

if(NOT _rc EQUAL 0)
    message(FATAL_ERROR
        "lpg2 failed (exit ${_rc}) LANG=${LANG} GRAMMAR=${GRAMMAR}\n"
        "stdout:\n${_out}\nstderr:\n${_err}")
endif()

# Map programming_language -> table file extension (see option.cpp help_get_file).
if(LANG STREQUAL "cpp" OR LANG STREQUAL "c" OR LANG STREQUAL "cpp2")
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
