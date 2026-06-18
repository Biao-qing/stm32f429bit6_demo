set(LWIP_ROOT ${CMAKE_SOURCE_DIR}/ThirdParty/LwIP)
set(LWIP_SRC ${LWIP_ROOT}/src)

if(NOT EXISTS "${LWIP_SRC}/include/lwip/opt.h")
    message(FATAL_ERROR "LwIP not found. Expected ThirdParty/LwIP from the repository.")
endif()

set(LWIP_CORE_SOURCES
    ${LWIP_SRC}/core/init.c
    ${LWIP_SRC}/core/def.c
    ${LWIP_SRC}/core/dns.c
    ${LWIP_SRC}/core/inet_chksum.c
    ${LWIP_SRC}/core/ip.c
    ${LWIP_SRC}/core/mem.c
    ${LWIP_SRC}/core/memp.c
    ${LWIP_SRC}/core/netif.c
    ${LWIP_SRC}/core/pbuf.c
    ${LWIP_SRC}/core/raw.c
    ${LWIP_SRC}/core/stats.c
    ${LWIP_SRC}/core/sys.c
    ${LWIP_SRC}/core/altcp.c
    ${LWIP_SRC}/core/altcp_alloc.c
    ${LWIP_SRC}/core/altcp_tcp.c
    ${LWIP_SRC}/core/tcp.c
    ${LWIP_SRC}/core/tcp_in.c
    ${LWIP_SRC}/core/tcp_out.c
    ${LWIP_SRC}/core/timeouts.c
    ${LWIP_SRC}/core/udp.c
    ${LWIP_SRC}/core/ipv4/autoip.c
    ${LWIP_SRC}/core/ipv4/dhcp.c
    ${LWIP_SRC}/core/ipv4/etharp.c
    ${LWIP_SRC}/core/ipv4/icmp.c
    ${LWIP_SRC}/core/ipv4/ip4_frag.c
    ${LWIP_SRC}/core/ipv4/ip4.c
    ${LWIP_SRC}/core/ipv4/ip4_addr.c
    ${LWIP_SRC}/api/api_lib.c
    ${LWIP_SRC}/api/api_msg.c
    ${LWIP_SRC}/api/err.c
    ${LWIP_SRC}/api/if_api.c
    ${LWIP_SRC}/api/netifapi.c
    ${LWIP_SRC}/api/tcpip.c
    ${LWIP_SRC}/netif/ethernet.c
)

add_library(lwip OBJECT ${LWIP_CORE_SOURCES})

target_include_directories(lwip PUBLIC
    ${CMAKE_SOURCE_DIR}/Platform/Port
    ${LWIP_SRC}/include
)

target_compile_definitions(lwip PUBLIC
    LWIP_TIMEVAL_PRIVATE=0
)

target_link_libraries(lwip PUBLIC stm32cubemx freertos_kernel)
