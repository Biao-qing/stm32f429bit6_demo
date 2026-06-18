#include "net_app.h"

#include "eth.h"
#include "ethernetif.h"

#include "lwip/tcpip.h"
#include "lwip/netif.h"
#include "lwip/dhcp.h"
#include "lwip/ip4_addr.h"

#include "FreeRTOS.h"
#include "task.h"

#define NET_BOOT_TASK_STACK  384U
#define NET_BOOT_TASK_PRIO   (tskIDLE_PRIORITY + 1)

static struct netif s_netif;
static volatile bool s_net_ready;

static void net_app_tcpip_init(void *arg)
{
  ip4_addr_t ipaddr;
  ip4_addr_t netmask;
  ip4_addr_t gateway;

  (void)arg;

  if (!MX_ETH_Init())
  {
    /* No PHY / wiring error: keep other tasks running. */
    return;
  }

  IP4_ADDR(&ipaddr, 0, 0, 0, 0);
  IP4_ADDR(&netmask, 0, 0, 0, 0);
  IP4_ADDR(&gateway, 0, 0, 0, 0);

  if (netif_add(&s_netif, &ipaddr, &netmask, &gateway, NULL, ethernetif_init,
                tcpip_input) == NULL)
  {
    return;
  }

  netif_set_default(&s_netif);

  if (netif_is_link_up(&s_netif))
  {
    netif_set_up(&s_netif);
    dhcp_start(&s_netif);
  }

  if (xTaskCreate(ethernetif_link_monitor, "EthLink", 384U, &s_netif,
                  (configMAX_PRIORITIES - 3), NULL) != pdPASS)
  {
    return;
  }

  s_net_ready = true;
}

static void net_boot_task(void *argument)
{
  (void)argument;

  /* Let flash_init and other tasks allocate heap first. */
  vTaskDelay(pdMS_TO_TICKS(200));

  tcpip_init(net_app_tcpip_init, NULL);
  vTaskDelete(NULL);
}

void NetApp_Init(void)
{
  BaseType_t ok;

  s_net_ready = false;
  ok = xTaskCreate(net_boot_task, "net_boot", NET_BOOT_TASK_STACK, NULL,
                   NET_BOOT_TASK_PRIO, NULL);
  if (ok != pdPASS)
  {
    return;
  }
}

bool NetApp_IsReady(void)
{
  return s_net_ready;
}
