# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2_w CACHE STRING "Board type")

# Random ChatGPT fix for following error: `Policy CMP0057 is not set: Support new IN_LIST if() operator`
# must be before pico_sdk_import.cmake is included
if(POLICY CMP0057)
    cmake_policy(SET CMP0057 NEW)
endif()

# Pull in Pico and FreeRTOS
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

if(PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

project(wolf_pico_examples C CXX ASM)

# Initialize the SDK
pico_sdk_init()

option(USE_UART "Output over UART instead of USB" OFF)

### Global Include Path
    include_directories(config)
    include_directories(include)
    include_directories(${PICO_SDK_PATH}/src/rp2_common/pico_lwip/include)
    include_directories(${PICO_SDK_PATH}/lib/lwip/contrib/ports/freertos/include)
    include_directories(${PICO_SDK_PATH}/lib/lwip/src/include)
    include_directories(${PICO_SDK_PATH}/src/rp2_common/pico_async_context/include)

    set(WOLFSSL_ROOT $ENV{WOLFSSL_ROOT})
    include_directories(${WOLFSSL_ROOT})
### End of Global Include Path


### wolfSSL/wolfCrypt library
    file(GLOB WOLFSSL_SRC
        "${WOLFSSL_ROOT}/src/*.c"
        "${WOLFSSL_ROOT}/wolfcrypt/src/*.c"
        "${WOLFSSL_ROOT}/wolfcrypt/src/port/rpi_pico/*"
    )
    list(REMOVE_ITEM WOLFSSL_SRC EXCLUDE REGEX
        "${WOLFSSL_ROOT}/src/bio.c"
        "${WOLFSSL_ROOT}/src/conf.c"
        "${WOLFSSL_ROOT}/src/pk.c"
        "${WOLFSSL_ROOT}/src/ssl_asn1.c"
        "${WOLFSSL_ROOT}/src/ssl_bn.c"
        "${WOLFSSL_ROOT}/src/ssl_misc.c"
        "${WOLFSSL_ROOT}/src/x509.c"
        "${WOLFSSL_ROOT}/src/x509_str.c"
        "${WOLFSSL_ROOT}/wolfcrypt/src/evp.c"
        "${WOLFSSL_ROOT}/wolfcrypt/src/misc.c"
    )

    add_library(wolfssl STATIC
        ${WOLFSSL_SRC}
    )

    target_compile_definitions(wolfssl PUBLIC
        WOLFSSL_USER_SETTINGS
    )
    if (${PICO_PLATFORM} STREQUAL "rp2350-arm-s")
        add_compile_definitions(wolfssl WOLFSSL_SP_ARM_CORTEX_M_ASM)
    elseif (${PICO_PLATFORM} STREQUAL "rp2350-riscv")
        add_compile_definitions(wolfSSL WOLFSSL_SP_RISCV32)
    else()
        add_compile_definitions(wolfssl WOLFSSL_SP_ARM_THUMB_ASM)
    endif()

    target_link_libraries(wolfssl
        pico_stdlib
        pico_rand
    )
### End of wolfSSL/wolfCrypt library


### Test wolfCrypt algorithms
    add_executable(testwolfcrypt
        src/test_main.c
        ${WOLFSSL_ROOT}/wolfcrypt/test/test.c
    )

    target_link_libraries(testwolfcrypt
        wolfssl
        pico_stdlib
        pico_rand
    )

    target_include_directories(testwolfcrypt PRIVATE ${WOLFSSL_ROOT})

    if (USE_UART)
        pico_enable_stdio_usb(testwolfcrypt 0)
        pico_enable_stdio_uart(testwolfcrypt 1)
        else()
        pico_enable_stdio_usb(testwolfcrypt 1)
        pico_enable_stdio_uart(testwolfcrypt 0)
        message(STATUS "We are using USB!")
    endif()

    if (${PICO_PLATFORM} STREQUAL "rp2350")
        add_compile_definitions(wolfssl WOLFSSL_SP_ARM_CORTEX_M_ASM)
        message(STATUS "We are using rp2350!")
    elseif (${PICO_PLATFORM} STREQUAL "rp2350-riscv")
        add_compile_definitions(wolfSSL WOLFSSL_SP_RISCV32)
    else()
        add_compile_definitions(wolfssl WOLFSSL_SP_ARM_THUMB_ASM)
    endif()

    pico_add_extra_outputs(testwolfcrypt)
### End of Test wolfCrypt algorithms