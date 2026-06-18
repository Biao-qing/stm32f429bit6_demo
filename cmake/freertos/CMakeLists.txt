set(FREERTOS_KERNEL_DIR ${CMAKE_SOURCE_DIR}/ThirdParty/FreeRTOS/Source)

set(FREERTOS_KERNEL_SOURCES
    ${FREERTOS_KERNEL_DIR}/croutine.c
    ${FREERTOS_KERNEL_DIR}/event_groups.c
    ${FREERTOS_KERNEL_DIR}/list.c
    ${FREERTOS_KERNEL_DIR}/queue.c
    ${FREERTOS_KERNEL_DIR}/stream_buffer.c
    ${FREERTOS_KERNEL_DIR}/tasks.c
    ${FREERTOS_KERNEL_DIR}/timers.c
    ${FREERTOS_KERNEL_DIR}/portable/GCC/ARM_CM4F/port.c
    ${FREERTOS_KERNEL_DIR}/portable/MemMang/heap_4.c
)

add_library(freertos_kernel OBJECT ${FREERTOS_KERNEL_SOURCES})

target_include_directories(freertos_kernel PUBLIC
    ${FREERTOS_KERNEL_DIR}/include
    ${FREERTOS_KERNEL_DIR}/portable/GCC/ARM_CM4F
    ${CMAKE_SOURCE_DIR}/Core/Inc
)

target_link_libraries(freertos_kernel PUBLIC stm32cubemx)
