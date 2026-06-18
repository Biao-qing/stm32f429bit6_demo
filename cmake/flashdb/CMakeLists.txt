set(FLASHDB_ROOT ${CMAKE_SOURCE_DIR}/ThirdParty/FlashDB)

set(FLASHDB_SOURCES
    ${FLASHDB_ROOT}/src/fdb.c
    ${FLASHDB_ROOT}/src/fdb_kvdb.c
    ${FLASHDB_ROOT}/src/fdb_tsdb.c
    ${FLASHDB_ROOT}/src/fdb_utils.c
    ${FLASHDB_ROOT}/port/fal/src/fal.c
    ${FLASHDB_ROOT}/port/fal/src/fal_flash.c
    ${FLASHDB_ROOT}/port/fal/src/fal_partition.c
    ${CMAKE_SOURCE_DIR}/Platform/Port/fal_flash_w25q64_port.c
)

add_library(flashdb OBJECT ${FLASHDB_SOURCES})

target_include_directories(flashdb PUBLIC
    ${CMAKE_SOURCE_DIR}/Platform/Port
    ${CMAKE_SOURCE_DIR}/Device/Inc
    ${CMAKE_SOURCE_DIR}/Core/Inc
    ${FLASHDB_ROOT}/inc
    ${FLASHDB_ROOT}/port/fal/inc
)

target_link_libraries(flashdb PUBLIC device_drv stm32cubemx)
