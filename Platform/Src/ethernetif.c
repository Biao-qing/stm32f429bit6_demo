#include "ethernetif.h"

#include "eth.h"
#include "lan8720.h"

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/stats.h"
#include "lwip/tcpip.h"
#include "lwip/netifapi.h"
#include "netif/ethernet.h"
#include "netif/etharp.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include <string.h>

#define IFNAME0                 's'
#define IFNAME1                 't'
#define ETHIF_TX_TIMEOUT_MS     2000U
#define ETH_RX_BUFFER_CNT       8U
#define ETHIF_INPUT_STACK       768U
#define ETHIF_INPUT_PRIO        (configMAX_PRIORITIES - 3)
#define ETHIF_LINK_STACK        256U
#define ETHIF_LINK_PRIO         (configMAX_PRIORITIES - 3)

typedef enum
{
  RX_ALLOC_OK = 0,
  RX_ALLOC_ERROR
} RxAllocStatusTypeDef;

typedef struct
{
  struct pbuf_custom pbuf_custom;
  uint8_t buff[(ETH_RX_BUF_SIZE + 31U) & ~31U] __ALIGNED(32);
} RxBuff_t;

LWIP_MEMPOOL_DECLARE(RX_POOL, ETH_RX_BUFFER_CNT, sizeof(RxBuff_t), "Zero-copy RX PBUF pool");

static ETH_TxPacketConfig s_tx_config;
static SemaphoreHandle_t s_rx_pkt_sem;
static SemaphoreHandle_t s_tx_pkt_sem;
static RxAllocStatusTypeDef s_rx_alloc_status;

static void ethernetif_input_task(void *argument);
static err_t low_level_output(struct netif *netif, struct pbuf *p);
static struct pbuf *low_level_input(struct netif *netif);
static void low_level_apply_phy(struct netif *netif, LAN8720_PhyStateTypeDef phy_state);

static void lan8720_apply_mac_config(LAN8720_PhyStateTypeDef phy_state)
{
  ETH_MACConfigTypeDef mac_conf = {0};
  uint32_t duplex = ETH_FULLDUPLEX_MODE;
  uint32_t speed = ETH_SPEED_100M;

  switch (phy_state)
  {
  case LAN8720_PHY_100M_FULL:
    duplex = ETH_FULLDUPLEX_MODE;
    speed = ETH_SPEED_100M;
    break;
  case LAN8720_PHY_100M_HALF:
    duplex = ETH_HALFDUPLEX_MODE;
    speed = ETH_SPEED_100M;
    break;
  case LAN8720_PHY_10M_FULL:
    duplex = ETH_FULLDUPLEX_MODE;
    speed = ETH_SPEED_10M;
    break;
  case LAN8720_PHY_10M_HALF:
    duplex = ETH_HALFDUPLEX_MODE;
    speed = ETH_SPEED_10M;
    break;
  default:
    return;
  }

  HAL_ETH_GetMACConfig(&heth, &mac_conf);
  mac_conf.DuplexMode = duplex;
  mac_conf.Speed = speed;
  HAL_ETH_SetMACConfig(&heth, &mac_conf);
}

static void low_level_apply_phy(struct netif *netif, LAN8720_PhyStateTypeDef phy_state)
{
  if (phy_state == LAN8720_PHY_LINK_DOWN)
  {
    HAL_ETH_Stop_IT(&heth);
    netif_set_link_down(netif);
    netif_set_down(netif);
    return;
  }

  lan8720_apply_mac_config(phy_state);
  HAL_ETH_Start_IT(&heth);
  netif_set_up(netif);
  netif_set_link_up(netif);
}

static void low_level_init(struct netif *netif)
{
  LAN8720_PhyStateTypeDef phy_state;

  LWIP_MEMPOOL_INIT(RX_POOL);

  memset(&s_tx_config, 0, sizeof(s_tx_config));
  s_tx_config.Attributes = ETH_TX_PACKETS_FEATURES_CSUM | ETH_TX_PACKETS_FEATURES_CRCPAD;
  s_tx_config.ChecksumCtrl = ETH_CHECKSUM_IPHDR_PAYLOAD_INSERT_PHDR_CALC;
  s_tx_config.CRCPadCtrl = ETH_CRC_PAD_INSERT;

  s_rx_pkt_sem = xSemaphoreCreateBinary();
  s_tx_pkt_sem = xSemaphoreCreateBinary();
  LWIP_ASSERT("rx sem", s_rx_pkt_sem != NULL);
  LWIP_ASSERT("tx sem", s_tx_pkt_sem != NULL);

  netif->hwaddr_len = ETH_HWADDR_LEN;
  memcpy(netif->hwaddr, heth.Init.MACAddr, ETH_HWADDR_LEN);
  netif->mtu = ETH_MAX_PAYLOAD;
  netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

  if (xTaskCreate(ethernetif_input_task, "EthIf", ETHIF_INPUT_STACK, netif,
                  ETHIF_INPUT_PRIO, NULL) != pdPASS)
  {
    return;
  }

  phy_state = LAN8720_GetPhyState(&heth);
  low_level_apply_phy(netif, phy_state);
}

static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
  uint32_t idx = 0U;
  struct pbuf *q = NULL;
  err_t errval = ERR_OK;
  ETH_BufferTypeDef tx_buffer[ETH_TX_DESC_CNT] = {0};

  (void)netif;

  for (q = p; q != NULL; q = q->next)
  {
    if (idx >= ETH_TX_DESC_CNT)
    {
      return ERR_IF;
    }

    tx_buffer[idx].buffer = q->payload;
    tx_buffer[idx].len = q->len;
    tx_buffer[idx].next = (idx + 1U < ETH_TX_DESC_CNT) ? &tx_buffer[idx + 1U] : NULL;
    idx++;
  }

  if (idx > 0U)
  {
    tx_buffer[idx - 1U].next = NULL;
  }

  s_tx_config.Length = p->tot_len;
  s_tx_config.TxBuffer = tx_buffer;
  s_tx_config.pData = p;
  pbuf_ref(p);

  do
  {
    if (HAL_ETH_Transmit_IT(&heth, &s_tx_config) == HAL_OK)
    {
      errval = ERR_OK;
    }
    else if ((HAL_ETH_GetError(&heth) & HAL_ETH_ERROR_BUSY) != 0U)
    {
      (void)xSemaphoreTake(s_tx_pkt_sem, pdMS_TO_TICKS(ETHIF_TX_TIMEOUT_MS));
      HAL_ETH_ReleaseTxPacket(&heth);
      errval = ERR_BUF;
    }
    else
    {
      pbuf_free(p);
      errval = ERR_IF;
    }
  } while (errval == ERR_BUF);

  return errval;
}

static struct pbuf *low_level_input(struct netif *netif)
{
  struct pbuf *p = NULL;

  (void)netif;

  if (s_rx_alloc_status == RX_ALLOC_OK)
  {
    HAL_ETH_ReadData(&heth, (void **)&p);
  }

  return p;
}

static void ethernetif_input_task(void *argument)
{
  struct pbuf *p = NULL;
  struct netif *netif = (struct netif *)argument;

  for (;;)
  {
    if (xSemaphoreTake(s_rx_pkt_sem, portMAX_DELAY) == pdTRUE)
    {
      do
      {
        p = low_level_input(netif);
        if (p != NULL)
        {
          if (netif->input(p, netif) != ERR_OK)
          {
            pbuf_free(p);
          }
        }
      } while (p != NULL);
    }
  }
}

err_t ethernetif_init(struct netif *netif)
{
  LWIP_ASSERT("netif != NULL", netif != NULL);

  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
  netif->output = etharp_output;
  netif->linkoutput = low_level_output;

  low_level_init(netif);
  return ERR_OK;
}

void ethernetif_link_monitor(void *argument)
{
  struct netif *netif = (struct netif *)argument;
  LAN8720_PhyStateTypeDef phy_state;

  for (;;)
  {
    phy_state = LAN8720_GetPhyState(&heth);

    if (netif_is_link_up(netif) && (phy_state == LAN8720_PHY_LINK_DOWN))
    {
      netifapi_netif_set_link_down(netif);
      netifapi_netif_set_down(netif);
      HAL_ETH_Stop_IT(&heth);
    }
    else if (!netif_is_link_up(netif) && (phy_state != LAN8720_PHY_LINK_DOWN))
    {
      lan8720_apply_mac_config(phy_state);
      HAL_ETH_Start_IT(&heth);
      netifapi_netif_set_up(netif);
      netifapi_netif_set_link_up(netif);
      if (netif->ip_addr.addr == 0U)
      {
        dhcp_start(netif);
      }
    }

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void pbuf_free_custom(struct pbuf *p)
{
  struct pbuf_custom *custom_pbuf = (struct pbuf_custom *)p;

  LWIP_MEMPOOL_FREE(RX_POOL, custom_pbuf);

  if (s_rx_alloc_status == RX_ALLOC_ERROR)
  {
    s_rx_alloc_status = RX_ALLOC_OK;
    xSemaphoreGive(s_rx_pkt_sem);
  }
}

void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef *eth_handle)
{
  (void)eth_handle;
  BaseType_t woken = pdFALSE;

  xSemaphoreGiveFromISR(s_rx_pkt_sem, &woken);
  portYIELD_FROM_ISR(woken);
}

void HAL_ETH_TxCpltCallback(ETH_HandleTypeDef *eth_handle)
{
  (void)eth_handle;
  BaseType_t woken = pdFALSE;

  xSemaphoreGiveFromISR(s_tx_pkt_sem, &woken);
  portYIELD_FROM_ISR(woken);
}

void HAL_ETH_ErrorCallback(ETH_HandleTypeDef *eth_handle)
{
  if ((HAL_ETH_GetDMAError(eth_handle) & ETH_DMA_RX_BUFFER_UNAVAILABLE_FLAG) != 0U)
  {
    BaseType_t woken = pdFALSE;

    xSemaphoreGiveFromISR(s_rx_pkt_sem, &woken);
    portYIELD_FROM_ISR(woken);
  }
}

void HAL_ETH_RxAllocateCallback(uint8_t **buff)
{
  struct pbuf_custom *p = LWIP_MEMPOOL_ALLOC(RX_POOL);

  if (p != NULL)
  {
    *buff = (uint8_t *)p + offsetof(RxBuff_t, buff);
    p->custom_free_function = pbuf_free_custom;
    pbuf_alloced_custom(PBUF_RAW, 0, PBUF_REF, p, *buff, ETH_RX_BUF_SIZE);
  }
  else
  {
    s_rx_alloc_status = RX_ALLOC_ERROR;
    *buff = NULL;
  }
}

void HAL_ETH_RxLinkCallback(void **pstart, void **pend, uint8_t *buff, uint16_t length)
{
  struct pbuf **pp_start = (struct pbuf **)pstart;
  struct pbuf **pp_end = (struct pbuf **)pend;
  struct pbuf *p = (struct pbuf *)(buff - offsetof(RxBuff_t, buff));

  p->next = NULL;
  p->tot_len = 0;
  p->len = length;

  if (*pp_start == NULL)
  {
    *pp_start = p;
  }
  else
  {
    (*pp_end)->next = p;
  }

  *pp_end = p;

  for (p = *pp_start; p != NULL; p = p->next)
  {
    p->tot_len += length;
  }
}

void HAL_ETH_TxFreeCallback(uint32_t *buff)
{
  pbuf_free((struct pbuf *)buff);
}
