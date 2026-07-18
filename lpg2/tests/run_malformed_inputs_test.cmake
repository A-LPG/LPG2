if(NOT DEFINED LPG2_EXE OR NOT DEFINED OUT_DIR)
    message(FATAL_ERROR "LPG2_EXE and OUT_DIR are required")
endif()

file(REMOVE_RECURSE "${OUT_DIR}")
file(MAKE_DIRECTORY "${OUT_DIR}")

# Each case must exit 12, emit a diagnostic, and leave no half-written tables.

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

set(_case_5 [=[
%Options programming_language=python2
%Terminals
    a
%Rules
    S ::= a
%End
]=])

set(_case_6 [=[
%Include
    "definitely/missing/include/file.gi"
%Terminals
    a
%Rules
    S ::= a
]=])

set(_case_7 [=[
%NotARealSection
    blah
%Terminals
    a
%Rules
    S ::= a
]=])

set(_case_8 [=[
%Terminals
    a
%Rules
%End
]=])

set(_case_9 [=[
%Terminals
    a
%Rules
    S ::= T
    T ::= U
    U ::= T
%End
]=])

set(_case_10 [=[
%Options fail_on_conflicts
%Terminals
    a PLUS EOF_TOKEN
%Eof
    EOF_TOKEN
%End
%Rules
    E ::= E PLUS E
        | a
%End
]=])

set(_case_11 [=[
%Terminals
    a
%Rules
    S ::= a /. return 1
]=])

set(_case_12 [=[
%Options programming_language=plxasm
%Terminals
    a
%Rules
    S ::= a
%End
]=])

set(_case_13 [=[
%End
%Terminals
    a
%Rules
    S ::= a
%End
]=])

set(_case_14 [=[
%Headers
 /. unterminated header block
%Terminals
    a
%Rules
    S ::= a
%End
]=])

set(_case_15 [=[
%Options soft_keywords, filter="no_such_filter.g"
%Terminals
    a
%Rules
    S ::= a
%End
]=])

set(_case_16 [=[
%Options automatic_ast=nested
%Terminals
    a EOF_TOKEN
%Eof
    EOF_TOKEN
%End
%Recover
    Missing /. new AstToken(
%Rules
    S ::= a
%End
]=])

set(_case_17 [=[
%Terminals
    a
%Rules
    S ::= a /. ((((( unfinished
]=])

set(_case_18 [=[
%Options programming_language=xml
%Terminals
    a
%Rules
    S ::= a
%End
]=])

set(_case_19 [=[
%Terminals
    a
%Rules
    S ::= a
%End
%Rules
    orphan after end ::= a
]=])

set(_case_20 [=[
%Include
    ""
%Terminals
    a
%Rules
    S ::= a
%End
]=])

set(_case_21 [=[
%Options table=none
%Terminals
    a
%Rules
    ::= a a a
%End
]=])

# "c" must not be accepted as a prefix of "cpp"/"csharp" (Release-only silent success).
set(_case_22 [=[
%Options programming_language=c
%Terminals
    a
%Rules
    S ::= a
%End
]=])

foreach(_index RANGE 0 22)
    set(_case_dir "${OUT_DIR}/case-${_index}")
    file(MAKE_DIRECTORY "${_case_dir}")
    set(_grammar "${_case_dir}/malformed.g")
    set(_case_variable "_case_${_index}")
    file(WRITE "${_grammar}" "${${_case_variable}}")

    set(_extra_args "")
    if(_index EQUAL 10)
        set(_extra_args -fail_on_conflicts)
    endif()

    execute_process(
        COMMAND "${LPG2_EXE}"
            -programming_language=cpp
            -table
            -quiet
            ${_extra_args}
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
