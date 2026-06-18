set(FLASHDB_ROOT ${CMAKE_SOURCE_DIR}/Vendor/FlashDB)

set(PLATFORM_SOURCES
    ${CMAKE_SOURCE_DIR}/Platform/Src/fdb_port.c
)

add_library(platform OBJECT ${PLATFORM_SOURCES})

target_include_directories(platform PUBLIC
    ${CMAKE_SOURCE_DIR}/Platform/Inc
    ${CMAKE_SOURCE_DIR}/Platform/Port
    ${CMAKE_SOURCE_DIR}/Core/Inc
    ${FLASHDB_ROOT}/inc
    ${FLASHDB_ROOT}/port/fal/inc
)

target_link_libraries(platform PUBLIC stm32cubemx freertos_kernel)
