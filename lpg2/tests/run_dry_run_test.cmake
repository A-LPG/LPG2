# Verify --dry-run is an alias for -nowrite: analysis only, no published files.
# Conflict mode (no -quiet) must still emit human-readable diagnostics on stderr/stdout.
if(NOT DEFINED LPG2_EXE OR NOT DEFINED GRAMMAR OR NOT DEFINED WORK_DIR
        OR NOT DEFINED MODE)
    message(FATAL_ERROR
        "LPG2_EXE, GRAMMAR, WORK_DIR, and MODE are required")
endif()

file(REMOVE_RECURSE "${WORK_DIR}")
file(MAKE_DIRECTORY "${WORK_DIR}")

if(MODE STREQUAL "json")
    set(_args
        --diagnostics=json
        --dry-run
        -quiet
        -programming_language=cpp
        -table
        -fail_on_conflicts
        "${GRAMMAR}")
    execute_process(
        COMMAND "${LPG2_EXE}" ${_args}
        WORKING_DIRECTORY "${WORK_DIR}"
        RESULT_VARIABLE _rc
        OUTPUT_VARIABLE _stdout
        ERROR_VARIABLE _stderr)
    if(NOT _rc EQUAL 12)
        message(FATAL_ERROR
            "json dry-run: expected exit 12, got ${_rc}\n"
            "stdout:\n${_stdout}\nstderr:\n${_stderr}")
    endif()
    if(NOT _stdout MATCHES "\"write_enabled\":false")
        message(FATAL_ERROR
            "json dry-run: expected write_enabled:false:\n${_stdout}")
    endif()
elseif(MODE STREQUAL "human_conflict")
    set(_args
        --dry-run
        -programming_language=java
        -table
        -fail_on_conflicts
        "${GRAMMAR}")
    execute_process(
        COMMAND "${LPG2_EXE}" ${_args}
        WORKING_DIRECTORY "${WORK_DIR}"
        RESULT_VARIABLE _rc
        OUTPUT_VARIABLE _stdout
        ERROR_VARIABLE _stderr)
    if(NOT _rc EQUAL 12)
        message(FATAL_ERROR
            "human_conflict: expected exit 12, got ${_rc}\n"
            "stdout:\n${_stdout}\nstderr:\n${_stderr}")
    endif()
    set(_combined "${_stdout}\n${_stderr}")
    if(NOT _combined MATCHES "[Cc]onflict|shift.reduce|reduce.reduce|LPG[0-9]+")
        message(FATAL_ERROR
            "human_conflict: expected human-readable conflict diagnostics:\n${_combined}")
    endif()
else()
    message(FATAL_ERROR "Unknown MODE=${MODE}")
endif()

file(GLOB_RECURSE _written_files "${WORK_DIR}/*")
# Only the empty work dir itself (or nothing); no generated sources / listings.
set(_unexpected "")
foreach(_f IN LISTS _written_files)
    if(IS_DIRECTORY "${_f}")
        continue()
    endif()
    list(APPEND _unexpected "${_f}")
endforeach()
if(_unexpected)
    message(FATAL_ERROR
        "${MODE}: --dry-run wrote files: ${_unexpected}")
endif()
