# Compare per-backend AST S-expression dumps against a golden shape.
#
# Required:
#   EXPECTED_DUMP - path to golden .sexpr
#   DUMP_FILES - produced ast.sexpr paths separated by '|' (not ';', which
#                breaks cmake -D argument parsing)
#
# At least two dump files must exist so the test is a real cross-backend check.
# Missing optional backends are skipped (not an error).

if(NOT DEFINED EXPECTED_DUMP OR NOT EXISTS "${EXPECTED_DUMP}")
    message(FATAL_ERROR "EXPECTED_DUMP missing: '${EXPECTED_DUMP}'")
endif()

file(READ "${EXPECTED_DUMP}" _expected)
string(STRIP "${_expected}" _expected)

set(_dumps "")
if(DEFINED DUMP_FILES)
    string(REPLACE "|" ";" _dump_list "${DUMP_FILES}")
    foreach(_f IN LISTS _dump_list)
        if(EXISTS "${_f}")
            list(APPEND _dumps "${_f}")
        endif()
    endforeach()
endif()

list(LENGTH _dumps _n)
if(_n LESS 2)
    # Not a failure: automatic_ast e2e tests (and their dumps) are only
    # registered when the corresponding language runtimes are available.
    # Bare CI (generator-only) therefore has 0 dumps; skip cleanly.
    message(STATUS
        "Skipping AST shape diff: need >= 2 dump files, found ${_n} "
        "(backends not built/run). DUMP_FILES=${DUMP_FILES}")
    return()
endif()

foreach(_f IN LISTS _dumps)
    file(READ "${_f}" _got)
    string(STRIP "${_got}" _got)
    if(NOT _got STREQUAL _expected)
        message(FATAL_ERROR
            "AST dump mismatch\n"
            "  file: ${_f}\n"
            "  expected: ${_expected}\n"
            "  got:      ${_got}")
    endif()
endforeach()

message(STATUS "AST shape OK (${_n} backends): ${_expected}")
