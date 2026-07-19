# -nowrite / --dry-run must analyze grammars in a read-only directory
# without opening sibling product temps (exit 12 / LpgError on fopen).
if(NOT DEFINED LPG2_EXE OR NOT DEFINED GRAMMAR OR NOT DEFINED WORK_DIR)
    message(FATAL_ERROR "LPG2_EXE, GRAMMAR, and WORK_DIR are required")
endif()

file(REMOVE_RECURSE "${WORK_DIR}")
file(MAKE_DIRECTORY "${WORK_DIR}")
file(COPY "${GRAMMAR}" DESTINATION "${WORK_DIR}")
get_filename_component(_grammar_name "${GRAMMAR}" NAME)
set(_grammar_copy "${WORK_DIR}/${_grammar_name}")

# Directory read-only; grammar file stays readable.
if(CMAKE_HOST_UNIX)
    execute_process(COMMAND chmod a-w "${WORK_DIR}" RESULT_VARIABLE _chmod_rc)
    if(NOT _chmod_rc EQUAL 0)
        message(FATAL_ERROR "chmod a-w failed on ${WORK_DIR}")
    endif()
endif()

execute_process(
    COMMAND "${LPG2_EXE}"
        --diagnostics=json
        -nowrite
        -quiet
        -programming_language=cpp
        -table
        -backtrack
        "${_grammar_copy}"
    WORKING_DIRECTORY "${WORK_DIR}"
    RESULT_VARIABLE _rc
    OUTPUT_VARIABLE _stdout
    ERROR_VARIABLE _stderr)

if(CMAKE_HOST_UNIX)
    execute_process(COMMAND chmod u+w "${WORK_DIR}")
endif()

if(NOT _rc EQUAL 0)
    message(FATAL_ERROR
        "readonly nowrite: expected exit 0, got ${_rc}\n"
        "stdout:\n${_stdout}\nstderr:\n${_stderr}")
endif()
if(NOT _stdout MATCHES "\"write_enabled\":false")
    message(FATAL_ERROR
        "readonly nowrite: expected write_enabled:false:\n${_stdout}")
endif()

file(GLOB _temps "${WORK_DIR}/*.lpg2-tmp-*")
if(_temps)
    message(FATAL_ERROR
        "readonly nowrite: created sibling temps: ${_temps}")
endif()
