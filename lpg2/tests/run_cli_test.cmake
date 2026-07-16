if(NOT DEFINED LPG2_EXE OR NOT DEFINED GRAMMAR OR NOT DEFINED OUT_DIR)
    message(FATAL_ERROR "LPG2_EXE, GRAMMAR, and OUT_DIR are required")
endif()

file(REMOVE_RECURSE "${OUT_DIR}")
file(MAKE_DIRECTORY "${OUT_DIR}/work" "${OUT_DIR}/generated")

execute_process(
    COMMAND "${LPG2_EXE}"
    RESULT_VARIABLE _rc
    OUTPUT_VARIABLE _stdout
    ERROR_VARIABLE _stderr)
if(NOT _rc EQUAL 0)
    message(FATAL_ERROR "No-argument help returned ${_rc}, expected 0")
endif()

foreach(_argument IN ITEMS "--help" "-help" "-h")
    execute_process(
        COMMAND "${LPG2_EXE}" "${_argument}"
        RESULT_VARIABLE _rc
        OUTPUT_VARIABLE _stdout
        ERROR_VARIABLE _stderr)
    if(NOT _rc EQUAL 0)
        message(FATAL_ERROR "${_argument} returned ${_rc}, expected 0")
    endif()
endforeach()

execute_process(
    COMMAND "${LPG2_EXE}" --version
    RESULT_VARIABLE _rc
    OUTPUT_VARIABLE _stdout
    ERROR_VARIABLE _stderr)
if(NOT _rc EQUAL 0 OR NOT _stdout MATCHES "2\\.3\\.0")
    message(FATAL_ERROR
        "--version failed (${_rc})\nstdout:\n${_stdout}\nstderr:\n${_stderr}")
endif()

execute_process(
    COMMAND "${LPG2_EXE}"
        -programming_language=cpp
        -table
        "-out_directory=${OUT_DIR}/generated"
        "${GRAMMAR}"
    WORKING_DIRECTORY "${OUT_DIR}/work"
    RESULT_VARIABLE _rc
    OUTPUT_VARIABLE _stdout
    ERROR_VARIABLE _stderr)
if(NOT _rc EQUAL 0)
    message(FATAL_ERROR
        "CLI generation failed (${_rc})\nstdout:\n${_stdout}\nstderr:\n${_stderr}")
endif()
if(NOT _stdout MATCHES "Generated [1-9][0-9]* file")
    message(FATAL_ERROR "Generation summary missing from stdout:\n${_stdout}")
endif()
if(NOT EXISTS "${OUT_DIR}/generated/minimal.l")
    message(FATAL_ERROR "-out_directory did not contain minimal.l")
endif()
if(EXISTS "${OUT_DIR}/work/minimal.l")
    message(FATAL_ERROR "Listing file leaked outside -out_directory")
endif()

execute_process(
    COMMAND "${LPG2_EXE}"
        -programming_language=notalang
        -quiet
        "-out_directory=${OUT_DIR}/generated"
        "${GRAMMAR}"
    WORKING_DIRECTORY "${OUT_DIR}/work"
    RESULT_VARIABLE _rc
    OUTPUT_VARIABLE _stdout
    ERROR_VARIABLE _stderr)
if(NOT _rc EQUAL 12)
    message(FATAL_ERROR "Invalid option returned ${_rc}, expected 12")
endif()
if(NOT _stderr MATCHES "Error:")
    message(FATAL_ERROR "Invalid option diagnostic was not written to stderr")
endif()
