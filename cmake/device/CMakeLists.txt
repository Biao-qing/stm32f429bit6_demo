set(DEVICE_SOURCES
    ${CMAKE_SOURCE_DIR}/Device/Src/w25q64.c
)

add_library(device_drv OBJECT ${DEVICE_SOURCES})

target_include_directories(device_drv PUBLIC
    ${CMAKE_SOURCE_DIR}/Device/Inc
    ${CMAKE_SOURCE_DIR}/Core/Inc
)

target_link_libraries(device_drv PUBLIC stm32cubemx)
