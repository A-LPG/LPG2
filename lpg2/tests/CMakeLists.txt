set(LPG2_TEST_RUNNER "${CMAKE_CURRENT_SOURCE_DIR}/run_generation_test.cmake")
set(LPG2_FIXTURES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/fixtures")
set(LPG2_CTEST_OUT_ROOT "${CMAKE_BINARY_DIR}/ctest")

# lpg2_add_generation_test(<name> <grammar> <lang> [LABELS ...] [EXPECT_FAIL] [PREFIX <prefix>])
function(lpg2_add_generation_test NAME GRAMMAR LANG)
    set(options EXPECT_FAIL)
    set(oneValueArgs PREFIX)
    set(multiValueArgs LABELS)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(_out_dir "${LPG2_CTEST_OUT_ROOT}/${NAME}")
    # All -D definitions must appear before -P (cmake script mode).
    set(_cmd
        ${CMAKE_COMMAND}
        "-DLPG2_EXE=$<TARGET_FILE:lpg2>"
        "-DGRAMMAR=${GRAMMAR}"
        "-DLANG=${LANG}"
        "-DOUT_DIR=${_out_dir}"
    )
    if(ARG_PREFIX)
        list(APPEND _cmd "-DEXPECT_PREFIX=${ARG_PREFIX}")
    endif()
    if(ARG_EXPECT_FAIL)
        list(APPEND _cmd "-DEXPECT_FAIL=TRUE")
    endif()
    list(APPEND _cmd -P "${LPG2_TEST_RUNNER}")

    add_test(NAME ${NAME} COMMAND ${_cmd})
    if(ARG_LABELS)
        set_tests_properties(${NAME} PROPERTIES LABELS "${ARG_LABELS}")
    endif()
endfunction()

# --- Tier A: fast multi-backend smoke (minimal.g) ---
set(_minimal "${LPG2_FIXTURES_DIR}/minimal.g")
foreach(_lang IN ITEMS cpp java go python3 csharp typescript dart rust)
    lpg2_add_generation_test(minimal_${_lang} "${_minimal}" ${_lang} LABELS smoke)
endforeach()

# --- Tier B: self-hosting bootstrap integration ---
set(_jikespg "${CMAKE_SOURCE_DIR}/grammar/jikespg.g")
# jikespg.g emits jikespg_prs.* / jikespg_sym.* (underscore after prefix).
lpg2_add_generation_test(bootstrap_cpp "${_jikespg}" cpp
    PREFIX jikespg_ LABELS integration)
lpg2_add_generation_test(bootstrap_rust "${_jikespg}" rust
    PREFIX jikespg_ LABELS integration)

# --- Tier C: feature / error-path regression ---
lpg2_add_generation_test(dropactions_import
    "${LPG2_FIXTURES_DIR}/import_dropactions.g" cpp
    PREFIX import_dropactions LABELS smoke feature)

lpg2_add_generation_test(invalid_lang_fails
    "${_minimal}" notalang
    EXPECT_FAIL LABELS smoke feature)
