set(ISOTP_ROOT ${CMAKE_SOURCE_DIR}/ThirdParty/isotp-c)

if(NOT EXISTS "${ISOTP_ROOT}/isotp.c")
    message(FATAL_ERROR "isotp-c not found. Run setup-thirdparty.ps1 first.")
endif()

add_library(isotp OBJECT
    ${ISOTP_ROOT}/isotp.c
)

target_include_directories(isotp PUBLIC
    ${ISOTP_ROOT}
)

target_compile_definitions(isotp PUBLIC
    ISO_TP_USER_SEND_CAN_ARG
)

target_link_libraries(isotp PUBLIC stm32cubemx)
