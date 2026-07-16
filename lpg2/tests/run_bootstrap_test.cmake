# Build a stage-one LPG2 from freshly generated C++ bootstrap sources, then
# use that binary to generate stage two and compare the generated artifacts.

foreach(_required IN ITEMS
        LPG2_EXE SOURCE_DIR GRAMMAR WORK_DIR BINARY_NAME)
    if(NOT DEFINED ${_required})
        message(FATAL_ERROR "${_required} is required")
    endif()
endforeach()

set(_stage1 "${WORK_DIR}/stage1")
set(_stage2 "${WORK_DIR}/stage2")
set(_source "${WORK_DIR}/source")
set(_build "${WORK_DIR}/build")
set(_nested_output "${WORK_DIR}/bin")

file(REMOVE_RECURSE "${WORK_DIR}")
file(MAKE_DIRECTORY "${_stage1}" "${_stage2}" "${_source}")

function(run_generator EXE OUT_DIR RESULT_OUT DIAGNOSTICS_OUT)
    execute_process(
        COMMAND "${EXE}"
                -programming_language=cpp
                -quiet
                "-out_directory=${OUT_DIR}"
                "${GRAMMAR}"
        WORKING_DIRECTORY "${OUT_DIR}"
        RESULT_VARIABLE _rc
        OUTPUT_VARIABLE _stdout
        ERROR_VARIABLE _stderr)
    if(NOT _rc EQUAL 0)
        message(FATAL_ERROR
            "Bootstrap generation failed with ${EXE} (exit ${_rc})\n"
            "stdout:\n${_stdout}\nstderr:\n${_stderr}")
    endif()
    set(${RESULT_OUT} "${_rc}" PARENT_SCOPE)
    set(${DIAGNOSTICS_OUT} "${_stdout}\n${_stderr}" PARENT_SCOPE)
endfunction()

run_generator("${LPG2_EXE}" "${_stage1}" _stage1_rc _stage1_diagnostics)

set(_generated_sources
    jikespg_act.cpp
    jikespg_init.cpp
    jikespg_prs.cpp)
set(_generated_headers
    jikespg_act.h
    jikespg_prs.h
    jikespg_sym.h)
foreach(_file IN LISTS _generated_sources _generated_headers)
    if(NOT EXISTS "${_stage1}/${_file}")
        message(FATAL_ERROR "Stage one did not generate ${_file}")
    endif()
endforeach()

# Keep the nested source tree minimal and independent of the caller's build.
file(COPY "${SOURCE_DIR}/CMakeLists.txt" DESTINATION "${_source}")
file(COPY "${SOURCE_DIR}/src" DESTINATION "${_source}")
file(COPY "${SOURCE_DIR}/include" DESTINATION "${_source}")
foreach(_file IN LISTS _generated_sources)
    file(COPY "${_stage1}/${_file}" DESTINATION "${_source}/src")
endforeach()
foreach(_file IN LISTS _generated_headers)
    file(COPY "${_stage1}/${_file}" DESTINATION "${_source}/include")
endforeach()

set(_configure_cmd
    "${CMAKE_COMMAND}"
    -S "${_source}"
    -B "${_build}"
    -DBUILD_TESTING=OFF
    "-DLPG2_OUTPUT_DIR=${_nested_output}"
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

execute_process(
    COMMAND ${_configure_cmd}
    RESULT_VARIABLE _configure_rc
    OUTPUT_VARIABLE _configure_out
    ERROR_VARIABLE _configure_err)
if(NOT _configure_rc EQUAL 0)
    message(FATAL_ERROR
        "Failed to configure stage-one generator (exit ${_configure_rc})\n"
        "stdout:\n${_configure_out}\nstderr:\n${_configure_err}")
endif()

if(NOT DEFINED TEST_CONFIG OR "${TEST_CONFIG}" STREQUAL "")
    set(TEST_CONFIG Release)
endif()
execute_process(
    COMMAND "${CMAKE_COMMAND}" --build "${_build}"
            --config "${TEST_CONFIG}" --parallel
    RESULT_VARIABLE _build_rc
    OUTPUT_VARIABLE _build_out
    ERROR_VARIABLE _build_err)
if(NOT _build_rc EQUAL 0)
    message(FATAL_ERROR
        "Failed to build stage-one generator (exit ${_build_rc})\n"
        "stdout:\n${_build_out}\nstderr:\n${_build_err}")
endif()

set(_stage1_exe "${_nested_output}/${BINARY_NAME}${EXE_SUFFIX}")
if(NOT EXISTS "${_stage1_exe}")
    message(FATAL_ERROR
        "Stage-one generator not found at ${_stage1_exe}")
endif()

run_generator("${_stage1_exe}" "${_stage2}" _stage2_rc _stage2_diagnostics)

set(_mismatches)
foreach(_file IN LISTS _generated_sources _generated_headers)
    file(SHA256 "${_stage1}/${_file}" _stage1_hash)
    file(SHA256 "${_stage2}/${_file}" _stage2_hash)
    if(NOT _stage1_hash STREQUAL _stage2_hash)
        list(APPEND _mismatches
            "${_file}: stage1=${_stage1_hash}, stage2=${_stage2_hash}")
    endif()
endforeach()

function(extract_conflict_count DIAGNOSTICS RESULT)
    string(REGEX MATCH
        "contains ([0-9]+) shift/reduce conflicts"
        _conflict_summary "${DIAGNOSTICS}")
    if(CMAKE_MATCH_1)
        set(${RESULT} "${CMAKE_MATCH_1}" PARENT_SCOPE)
    else()
        set(${RESULT} "0" PARENT_SCOPE)
    endif()
endfunction()
extract_conflict_count("${_stage1_diagnostics}" _stage1_conflicts)
extract_conflict_count("${_stage2_diagnostics}" _stage2_conflicts)
if(NOT _stage1_conflicts STREQUAL _stage2_conflicts)
    list(APPEND _mismatches
        "shift/reduce conflicts: stage1=${_stage1_conflicts}, "
        "stage2=${_stage2_conflicts}")
endif()

if(_mismatches)
    list(JOIN _mismatches "\n  " _mismatch_report)
    message(FATAL_ERROR
        "Bootstrap drift detected:\n  ${_mismatch_report}\n"
        "Artifacts retained under ${WORK_DIR}")
endif()
