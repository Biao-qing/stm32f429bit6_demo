set(APPLICATION_SOURCES
    ${CMAKE_SOURCE_DIR}/Application/Src/app_freertos.c
    ${CMAKE_SOURCE_DIR}/Application/Src/flash_app.c
    ${CMAKE_SOURCE_DIR}/Application/Src/led_app.c
    ${CMAKE_SOURCE_DIR}/Application/Src/uart_app.c
)

add_library(application OBJECT ${APPLICATION_SOURCES})

target_include_directories(application PUBLIC
    ${CMAKE_SOURCE_DIR}/Application/Inc
    ${CMAKE_SOURCE_DIR}/Services/Inc
    ${CMAKE_SOURCE_DIR}/Core/Inc
)

target_link_libraries(application PUBLIC services stm32cubemx freertos_kernel)
