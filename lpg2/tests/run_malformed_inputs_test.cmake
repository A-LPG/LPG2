if(NOT DEFINED LPG2_EXE OR NOT DEFINED OUT_DIR)
    message(FATAL_ERROR "LPG2_EXE and OUT_DIR are required")
endif()

file(REMOVE_RECURSE "${OUT_DIR}")
file(MAKE_DIRECTORY "${OUT_DIR}")

set(_case_0 "")
set(_case_1 [=[
%Terminals
    a
%Rules
    S ::= a /. unterminated action
]=])
set(_case_2 [=[
%Terminals
    'unterminated
%Rules
    S ::= 'unterminated
]=])
set(_case_3 [=[
%Terminals
    a
%Rules
    ::= a
]=])
set(_case_4 [=[
%Terminals
    <unterminated
%Rules
    S ::= <unterminated
]=])

foreach(_index RANGE 0 4)
    set(_case_dir "${OUT_DIR}/case-${_index}")
    file(MAKE_DIRECTORY "${_case_dir}")
    set(_grammar "${_case_dir}/malformed.g")
    set(_case_variable "_case_${_index}")
    file(WRITE "${_grammar}" "${${_case_variable}}")

    execute_process(
        COMMAND "${LPG2_EXE}"
            -programming_language=cpp
            -table
            -quiet
            "-out_directory=${_case_dir}"
            "${_grammar}"
        WORKING_DIRECTORY "${_case_dir}"
        RESULT_VARIABLE _rc
        OUTPUT_VARIABLE _stdout
        ERROR_VARIABLE _stderr
    )

    if(NOT _rc EQUAL 12)
        message(FATAL_ERROR
            "Malformed case ${_index} returned ${_rc}, expected 12\n"
            "stdout:\n${_stdout}\nstderr:\n${_stderr}")
    endif()

    if("${_stdout}${_stderr}" STREQUAL "")
        message(FATAL_ERROR
            "Malformed case ${_index} did not emit a diagnostic")
    endif()

    file(GLOB _partial_outputs
        "${_case_dir}/*prs.*"
        "${_case_dir}/*sym.*"
        "${_case_dir}/*.lpg2-*")
    if(_partial_outputs)
        message(FATAL_ERROR
            "Malformed case ${_index} left partial outputs: ${_partial_outputs}")
    endif()
endforeach()
