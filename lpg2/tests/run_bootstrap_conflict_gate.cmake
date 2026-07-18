# Gate: regenerating the self-hosting grammar must not exceed the soft
# shift/reduce conflict threshold checked in for src/ (see BOOTSTRAP.md).
# This catches "blind promotion" drift before staging files overwrite src/.

if(NOT DEFINED LPG2_EXE OR NOT DEFINED WORK_DIR OR NOT DEFINED GRAMMAR_FILE
   OR NOT DEFINED SOFT_MAX_FILE)
    message(FATAL_ERROR
        "LPG2_EXE, WORK_DIR, GRAMMAR_FILE, and SOFT_MAX_FILE are required")
endif()

file(READ "${SOFT_MAX_FILE}" _soft_max_raw)
string(STRIP "${_soft_max_raw}" _soft_max)
if(NOT _soft_max MATCHES "^[0-9]+$")
    message(FATAL_ERROR
        "SOFT_MAX_FILE must contain a non-negative integer, got: '${_soft_max}'")
endif()

file(REMOVE_RECURSE "${WORK_DIR}")
file(MAKE_DIRECTORY "${WORK_DIR}")

execute_process(
    COMMAND "${LPG2_EXE}"
        -programming_language=cpp_legacy
        -table
        -quiet
        "-out_directory=${WORK_DIR}"
        "${GRAMMAR_FILE}"
    WORKING_DIRECTORY "${WORK_DIR}"
    RESULT_VARIABLE _rc
    OUTPUT_VARIABLE _stdout
    ERROR_VARIABLE _stderr
)

set(_combined "${_stdout}${_stderr}")
if(NOT _rc EQUAL 0)
    message(FATAL_ERROR
        "Bootstrap conflict gate: generator exited ${_rc}\n${_combined}")
endif()

set(_conflicts "")
if(_combined MATCHES "contains ([0-9]+) shift/reduce conflicts")
    set(_conflicts "${CMAKE_MATCH_1}")
elseif(_combined MATCHES "Number of Shift-Reduce conflicts: *([0-9]+)")
    set(_conflicts "${CMAKE_MATCH_1}")
else()
    # Listing file is the durable record when -quiet suppresses stdout.
    file(GLOB _listings "${WORK_DIR}/*.l")
    foreach(_listing IN LISTS _listings)
        file(READ "${_listing}" _listing_text)
        if(_listing_text MATCHES "contains ([0-9]+) shift/reduce conflicts")
            set(_conflicts "${CMAKE_MATCH_1}")
            break()
        elseif(_listing_text MATCHES "Number of Shift-Reduce conflicts: *([0-9]+)")
            set(_conflicts "${CMAKE_MATCH_1}")
            break()
        endif()
    endforeach()
endif()

if(_conflicts STREQUAL "")
    message(FATAL_ERROR
        "Bootstrap conflict gate: could not parse shift/reduce conflict count\n"
        "${_combined}")
endif()

if(_conflicts GREATER _soft_max)
    message(FATAL_ERROR
        "Bootstrap conflict gate FAILED: regenerated grammar reports "
        "${_conflicts} shift/reduce conflicts, soft max is ${_soft_max} "
        "(see BOOTSTRAP.md). Do not promote grammar/.lpg into src/ until "
        "conflicts are at or below the checked-in threshold.")
endif()

message(STATUS
    "Bootstrap conflict gate OK: ${_conflicts} <= soft max ${_soft_max}")
