set(J1939_ROOT ${CMAKE_SOURCE_DIR}/ThirdParty/Open-SAE-J1939)

if(NOT EXISTS "${J1939_ROOT}/Src/Open_SAE_J1939/Open_SAE_J1939.h")
    message(FATAL_ERROR "Open-SAE-J1939 not found. Run setup-thirdparty.ps1 first.")
endif()

file(GLOB_RECURSE J1939_SOURCES "${J1939_ROOT}/Src/*.c")
list(FILTER J1939_SOURCES EXCLUDE REGEX ".*/Main\\.c$")
list(FILTER J1939_SOURCES EXCLUDE REGEX ".*/SocketCAN_Transmit_Receive\\.c$")

add_library(j1939 OBJECT ${J1939_SOURCES})

target_include_directories(j1939 PUBLIC
    ${J1939_ROOT}/Src
)

target_compile_definitions(j1939 PUBLIC
    OPENSAE_J1939_TARGET_PLATFORM=INTERNAL_CALLBACK
)

target_link_libraries(j1939 PUBLIC stm32cubemx freertos_kernel)
