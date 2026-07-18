if(NOT DEFINED LPG2_EXE OR NOT DEFINED GRAMMAR OR NOT DEFINED WORK_DIR
        OR NOT DEFINED MODE)
    message(FATAL_ERROR
        "LPG2_EXE, GRAMMAR, WORK_DIR, and MODE are required")
endif()

file(REMOVE_RECURSE "${WORK_DIR}")
file(MAKE_DIRECTORY "${WORK_DIR}")

set(_args
    --diagnostics=json
    -nowrite
    -quiet
    -programming_language=cpp
    -table)
if(MODE STREQUAL "conflict")
    list(APPEND _args -fail_on_conflicts)
elseif(MODE STREQUAL "health")
    list(APPEND _args -backtrack)
elseif(MODE STREQUAL "glr")
    list(APPEND _args -glr -fail_on_conflicts)
endif()
list(APPEND _args "${GRAMMAR}")

execute_process(
    COMMAND "${LPG2_EXE}" ${_args}
    WORKING_DIRECTORY "${WORK_DIR}"
    RESULT_VARIABLE _rc
    OUTPUT_VARIABLE _stdout
    ERROR_VARIABLE _stderr)

if(MODE STREQUAL "health" OR MODE STREQUAL "glr")
    set(_expected_rc 0)
else()
    set(_expected_rc 12)
endif()
if(NOT _rc EQUAL _expected_rc)
    message(FATAL_ERROR
        "${MODE}: generator returned ${_rc}, expected ${_expected_rc}\n"
        "stdout:\n${_stdout}\nstderr:\n${_stderr}")
endif()
if(NOT _stderr STREQUAL "")
    message(FATAL_ERROR
        "${MODE}: JSON mode wrote non-JSON text to stderr:\n${_stderr}")
endif()
if(NOT _stdout MATCHES "^\\{.*\\}[\r\n]*$")
    message(FATAL_ERROR
        "${MODE}: stdout is not a single JSON object:\n${_stdout}")
endif()
foreach(_field IN ITEMS
        "\"schema_version\":1"
        "\"diagnostics\":\\["
        "\"code\":\"LPG[0-9]+\""
        "\"span\":\\{"
        "\"start\":\\{\"line\":[0-9]+,\"column\":[0-9]+,\"offset\":[0-9]+\\}"
        "\"severity\":\""
        "\"message\":\""
        "\"help\":"
        "\"health\":\\{"
        "\"conflict_count\":"
        "\"backtrack\":"
        "\"glr\":"
        "\"glr_template_hint\":"
        "\"soft_keywords\":"
        "\"recover_symbols\":\\["
        "\"programming_language\":\"rt_cpp\""
        "\"warning_summary\":\\{")
    if(NOT _stdout MATCHES "${_field}")
        message(FATAL_ERROR
            "${MODE}: JSON is missing field matching ${_field}:\n${_stdout}")
    endif()
endforeach()

if(MODE STREQUAL "bad")
    if(NOT _stdout MATCHES
            "\"severity\":\"error\".*\"help\":\"[^\"]+\"")
        message(FATAL_ERROR
            "bad: expected an error with non-empty help:\n${_stdout}")
    endif()
elseif(MODE STREQUAL "conflict")
    foreach(_field IN ITEMS
            "\"code\":\"LPG2001\""
            "\"conflict_kind\":\"shift/reduce\""
            "\"example_lookahead\":\"[^\"]+\""
            "\"conflict_count\":[1-9][0-9]*"
            "\"warnings\":[1-9][0-9]*")
        if(NOT _stdout MATCHES "${_field}")
            message(FATAL_ERROR
                "conflict: JSON is missing ${_field}:\n${_stdout}")
        endif()
    endforeach()
elseif(MODE STREQUAL "health")
    foreach(_field IN ITEMS
            "\"available\":true"
            "\"healthy\":true"
            "\"conflict_count\":0"
            "\"backtrack\":true"
            "\"recover_symbols\":\\[\"Missing\"\\]"
            "\"write_enabled\":false")
        if(NOT _stdout MATCHES "${_field}")
            message(FATAL_ERROR
                "health: JSON is missing ${_field}:\n${_stdout}")
        endif()
    endforeach()
elseif(MODE STREQUAL "glr")
    foreach(_field IN ITEMS
            "\"available\":true"
            "\"healthy\":true"
            "\"conflict_count\":[1-9][0-9]*"
            "\"backtrack\":true"
            "\"glr\":true"
            "\"glr_template_hint\":\"use -template="
            "\"write_enabled\":false")
        if(NOT _stdout MATCHES "${_field}")
            message(FATAL_ERROR
                "glr: JSON is missing ${_field}:\n${_stdout}")
        endif()
    endforeach()
else()
    message(FATAL_ERROR "Unknown diagnostics test MODE=${MODE}")
endif()

file(GLOB _written_files "${WORK_DIR}/*")
if(_written_files)
    message(FATAL_ERROR
        "${MODE}: -nowrite created files: ${_written_files}")
endif()
