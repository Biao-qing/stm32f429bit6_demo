set(FLASHDB_ROOT ${CMAKE_SOURCE_DIR}/ThirdParty/FlashDB)

set(PLATFORM_SOURCES
    ${CMAKE_SOURCE_DIR}/Platform/Src/fdb_port.c
    ${CMAKE_SOURCE_DIR}/Platform/Src/sys_arch.c
    ${CMAKE_SOURCE_DIR}/Platform/Src/ethernetif.c
    ${CMAKE_SOURCE_DIR}/Platform/Src/can_if.c
    ${CMAKE_SOURCE_DIR}/Platform/Src/can_user_j1939.c
    ${CMAKE_SOURCE_DIR}/Platform/Src/can_user_uds.c
    ${CMAKE_SOURCE_DIR}/Platform/Src/j1939_app.c
    ${CMAKE_SOURCE_DIR}/Platform/Src/uds_app.c
)

add_library(platform OBJECT ${PLATFORM_SOURCES})

target_include_directories(platform PUBLIC
    ${CMAKE_SOURCE_DIR}/Platform/Inc
    ${CMAKE_SOURCE_DIR}/Platform/Port
    ${CMAKE_SOURCE_DIR}/Core/Inc
    ${CMAKE_SOURCE_DIR}/Device/Inc
    ${FLASHDB_ROOT}/inc
    ${FLASHDB_ROOT}/port/fal/inc
)

target_link_libraries(platform PUBLIC stm32cubemx freertos_kernel lwip device_drv j1939 isotp)
