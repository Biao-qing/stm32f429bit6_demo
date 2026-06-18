#ifndef ETHERNETIF_H
#define ETHERNETIF_H

#include "lwip/netif.h"

err_t ethernetif_init(struct netif *netif);
void ethernetif_link_monitor(void *argument);

#endif /* ETHERNETIF_H */
