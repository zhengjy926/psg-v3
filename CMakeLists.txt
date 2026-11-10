cmake_minimum_required(VERSION 3.16)
project(psg_v3_host_tests C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ASan/UBSan require the sanitizer runtime libraries (libasan/libubsan), which
# ship with Linux GCC/Clang. Some Windows-hosted GCC toolchains (e.g. mingw-w64)
# omit them, so allow disabling the sanitizers there.
option(ENABLE_SANITIZERS "Build the host tests with ASan + UBSan" ON)

# ---------------------------------------------------------------------------
# Host test executable. Only the host-clean layers (Zephyr port under lib/,
# osal, service, app) plus test doubles are compiled here; board/ and the
# target drivers/ are excluded (they include STM32 headers).
# ---------------------------------------------------------------------------

set(ZEPHYR_SRC
    lib/utils/init_runner.c
    lib/utils/device.c
)

set(APP_UNDER_TEST
    osal/osal.c
    service/fault/fault.c
    service/heartbeat/heartbeat.c
    app/app.c
)

set(TEST_SRC
    tests/host/runner.c
    tests/host/fakes/fake_gpio.c
    tests/host/console_backend.c
    tests/host/assert_host.c
    tests/host/test_init_order.c
    tests/host/test_device.c
    tests/host/test_heartbeat.c
    tests/host/test_do_device_init.c
    tests/host/test_device_traversal.c
    tests/host/test_fault.c
    third_party/unity/src/unity.c
)

add_executable(host_tests ${ZEPHYR_SRC} ${APP_UNDER_TEST} ${TEST_SRC})

target_include_directories(host_tests PRIVATE
    app
    lib
    lib/utils
    lib/os
    lib/toolchain
    lib/libc/armstdc/include
    subsys
    drivers/include
    third_party/unity/src
    tests/host
    ${CMAKE_SOURCE_DIR}
)

target_compile_options(host_tests PRIVATE
    -Wall
    -Wextra
    -g
)

target_link_options(host_tests PRIVATE
    -T${CMAKE_SOURCE_DIR}/tests/host/init_sections.ld
)

if(ENABLE_SANITIZERS)
    target_compile_options(host_tests PRIVATE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
    )
    target_link_options(host_tests PRIVATE
        -fsanitize=address,undefined
    )
endif()

enable_testing()
add_test(NAME host_tests COMMAND host_tests)

# ---------------------------------------------------------------------------
# Hard constraint: the host-clean layers must not include STM32/MCU headers.
# Runs on every build; a violation fails the build.
# ---------------------------------------------------------------------------
find_package(Python3 COMPONENTS Interpreter REQUIRED)

add_custom_target(check_no_stm32 ALL
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/check_no_stm32.py
            ${CMAKE_SOURCE_DIR}/osal
            ${CMAKE_SOURCE_DIR}/service
            ${CMAKE_SOURCE_DIR}/app
            ${CMAKE_SOURCE_DIR}/lib/utils
    COMMENT "Checking host-clean layers for STM32/MCU headers"
    VERBATIM
)
