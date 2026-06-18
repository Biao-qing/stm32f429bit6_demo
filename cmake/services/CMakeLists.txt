set(FLASHDB_ROOT ${CMAKE_SOURCE_DIR}/ThirdParty/FlashDB)

set(SERVICE_SOURCES
    ${CMAKE_SOURCE_DIR}/Services/Src/flashdb_app.c
    ${CMAKE_SOURCE_DIR}/Services/Src/net_app.c
    ${CMAKE_SOURCE_DIR}/Services/Src/can_app.c
)

add_library(services OBJECT ${SERVICE_SOURCES})

target_include_directories(services PUBLIC
    ${CMAKE_SOURCE_DIR}/Services/Inc
    ${CMAKE_SOURCE_DIR}/Platform/Inc
    ${CMAKE_SOURCE_DIR}/Device/Inc
    ${CMAKE_SOURCE_DIR}/Core/Inc
    ${FLASHDB_ROOT}/inc
    ${FLASHDB_ROOT}/port/fal/inc
)

target_link_libraries(services PUBLIC flashdb platform device_drv stm32cubemx lwip)
