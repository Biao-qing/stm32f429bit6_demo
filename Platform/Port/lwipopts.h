#ifndef LWIPOPTS_H
#define LWIPOPTS_H

#include "FreeRTOSConfig.h"

#define NO_SYS                  0
#define SYS_LIGHTWEIGHT_PROT    1
#define LWIP_COMPAT_MUTEX       0

#define MEM_ALIGNMENT           4
#define MEM_SIZE                (10 * 1024)
#define MEMP_NUM_PBUF           10
#define MEMP_NUM_UDP_PCB        4
#define MEMP_NUM_TCP_PCB        5
#define MEMP_NUM_TCP_PCB_LISTEN 4
#define MEMP_NUM_TCP_SEG        16
#define MEMP_NUM_SYS_TIMEOUT    8

#define PBUF_POOL_SIZE          8
#define PBUF_POOL_BUFSIZE       1524

#define LWIP_TCP                1
#define TCP_TTL                 255
#define TCP_QUEUE_OOSEQ         0
#define TCP_MSS                 (1500 - 40)
#define TCP_SND_BUF             (4 * TCP_MSS)
#define TCP_WND                 (2 * TCP_MSS)

#define LWIP_ICMP               1
#define LWIP_UDP                1
#define UDP_TTL                 255
#define LWIP_DHCP               1
#define LWIP_DNS                0
#define LWIP_STATS              0

#define LWIP_NETIF_LINK_CALLBACK 1
#define LWIP_NETIF_STATUS_CALLBACK 0
#define LWIP_NETIF_HOSTNAME     0
#define LWIP_NETIF_API          1

#define LWIP_NETCONN            0
#define LWIP_SOCKET             0

#define CHECKSUM_BY_HARDWARE

#ifdef CHECKSUM_BY_HARDWARE
#define CHECKSUM_GEN_IP         0
#define CHECKSUM_GEN_UDP        0
#define CHECKSUM_GEN_TCP        0
#define CHECKSUM_CHECK_IP       0
#define CHECKSUM_CHECK_UDP      0
#define CHECKSUM_CHECK_TCP      0
#define CHECKSUM_GEN_ICMP       0
#else
#define CHECKSUM_GEN_IP         1
#define CHECKSUM_GEN_UDP        1
#define CHECKSUM_GEN_TCP        1
#define CHECKSUM_CHECK_IP       1
#define CHECKSUM_CHECK_UDP      1
#define CHECKSUM_CHECK_TCP      1
#define CHECKSUM_GEN_ICMP       1
#endif

#define TCPIP_THREAD_NAME       "tcpip"
#define TCPIP_THREAD_STACKSIZE  1536
#define TCPIP_MBOX_SIZE         6
#define TCPIP_THREAD_PRIO       (configMAX_PRIORITIES - 2)

#define DEFAULT_RAW_RECVMBOX_SIZE    6
#define DEFAULT_UDP_RECVMBOX_SIZE    6
#define DEFAULT_TCP_RECVMBOX_SIZE    6
#define DEFAULT_ACCEPTMBOX_SIZE      6

#endif /* LWIPOPTS_H */
