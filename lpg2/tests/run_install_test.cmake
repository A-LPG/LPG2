if(NOT DEFINED BUILD_DIR OR
   NOT DEFINED OUT_DIR OR
   NOT DEFINED GRAMMAR OR
   NOT DEFINED BINARY_NAME)
    message(FATAL_ERROR
        "BUILD_DIR, OUT_DIR, GRAMMAR, and BINARY_NAME are required")
endif()

file(REMOVE_RECURSE "${OUT_DIR}")
file(MAKE_DIRECTORY "${OUT_DIR}")

set(_install_command
    "${CMAKE_COMMAND}" --install "${BUILD_DIR}" --prefix "${OUT_DIR}/prefix")
if(DEFINED TEST_CONFIG AND NOT "${TEST_CONFIG}" STREQUAL "")
    list(APPEND _install_command --config "${TEST_CONFIG}")
endif()
execute_process(
    COMMAND ${_install_command}
    RESULT_VARIABLE _install_rc
    OUTPUT_VARIABLE _install_out
    ERROR_VARIABLE _install_err)
if(NOT _install_rc EQUAL 0)
    message(FATAL_ERROR
        "Install failed (${_install_rc})\n"
        "stdout:\n${_install_out}\nstderr:\n${_install_err}")
endif()

set(_binary "${OUT_DIR}/prefix/bin/${BINARY_NAME}${EXE_SUFFIX}")
set(_resource_root
    "${OUT_DIR}/prefix/share/lpg2/lpg-generator-templates-2.1.00")
if(NOT EXISTS "${_binary}" OR
   NOT EXISTS "${_resource_root}/templates/rt_cpp/dtParserTemplateF.gi")
    message(FATAL_ERROR "Installed binary or templates are missing")
endif()
if(NOT EXISTS
       "${_resource_root}/templates/java/glrParserTemplateF.gi")
    message(FATAL_ERROR
        "Installed Java GLR template is missing: "
        "${_resource_root}/templates/java/glrParserTemplateF.gi")
endif()

file(MAKE_DIRECTORY "${OUT_DIR}/generated")
# Clear inherited template search paths so install-prefix discovery is tested.
execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env
        --unset=LPG_TEMPLATE
        --unset=LPG_INCLUDE
        "${_binary}"
        -programming_language=cpp
        -template=dtParserTemplateF.gi
        -table
        -quiet
        "-out_directory=${OUT_DIR}/generated"
        "${GRAMMAR}"
    WORKING_DIRECTORY "${OUT_DIR}"
    RESULT_VARIABLE _generator_rc
    OUTPUT_VARIABLE _generator_out
    ERROR_VARIABLE _generator_err)
if(NOT _generator_rc EQUAL 0)
    message(FATAL_ERROR
        "Installed generator could not discover templates (${_generator_rc})\n"
        "stdout:\n${_generator_out}\nstderr:\n${_generator_err}")
endif()

if(DEFINED GLR_GRAMMAR AND NOT "${GLR_GRAMMAR}" STREQUAL "")
    file(MAKE_DIRECTORY "${OUT_DIR}/generated_glr")
    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E env
            --unset=LPG_TEMPLATE
            --unset=LPG_INCLUDE
            "${_binary}"
            -programming_language=java
            -template=glrParserTemplateF.gi
            -table
            -quiet
            -nowrite
            "-out_directory=${OUT_DIR}/generated_glr"
            "${GLR_GRAMMAR}"
        WORKING_DIRECTORY "${OUT_DIR}"
        RESULT_VARIABLE _glr_rc
        OUTPUT_VARIABLE _glr_out
        ERROR_VARIABLE _glr_err)
    if(NOT _glr_rc EQUAL 0)
        message(FATAL_ERROR
            "Installed generator could not use glrParserTemplateF.gi "
            "(${_glr_rc})\nstdout:\n${_glr_out}\nstderr:\n${_glr_err}")
    endif()
endif()
