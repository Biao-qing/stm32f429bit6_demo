set(APP_SOURCES
    ${CMAKE_SOURCE_DIR}/App/Src/app_freertos.c
    ${CMAKE_SOURCE_DIR}/App/Src/uart_app.c
)

add_library(app OBJECT ${APP_SOURCES})

target_include_directories(app PUBLIC
    ${CMAKE_SOURCE_DIR}/App/Inc
    ${CMAKE_SOURCE_DIR}/Core/Inc
)

target_link_libraries(app PUBLIC stm32cubemx freertos_kernel)
