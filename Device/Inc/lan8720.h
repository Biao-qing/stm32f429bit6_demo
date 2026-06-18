#ifndef LAN8720_H
#define LAN8720_H

#include "stm32f4xx_hal.h"

#define LAN8720_PHY_ADDRESS  0U

#define LAN8720_BCR           0x00U
#define LAN8720_BSR           0x01U
#define LAN8720_PHYSCSR       0x1FU

#define LAN8720_BCR_SOFT_RESET  0x8000U
#define LAN8720_BSR_LINK_STATUS 0x0004U

#define LAN8720_SPEED_STATUS    0x0004U
#define LAN8720_DUPLEX_STATUS   0x0010U

typedef enum
{
  LAN8720_STATUS_OK = 0,
  LAN8720_STATUS_ERROR
} LAN8720_StatusTypeDef;

typedef enum
{
  LAN8720_LINK_DOWN = 0,
  LAN8720_LINK_UP
} LAN8720_LinkStateTypeDef;

typedef enum
{
  LAN8720_PHY_LINK_DOWN = 0,
  LAN8720_PHY_100M_FULL,
  LAN8720_PHY_100M_HALF,
  LAN8720_PHY_10M_FULL,
  LAN8720_PHY_10M_HALF
} LAN8720_PhyStateTypeDef;

void LAN8720_Reset(void);
LAN8720_StatusTypeDef LAN8720_Init(ETH_HandleTypeDef *heth);
LAN8720_LinkStateTypeDef LAN8720_GetLinkState(ETH_HandleTypeDef *heth);
LAN8720_PhyStateTypeDef LAN8720_GetPhyState(ETH_HandleTypeDef *heth);

#endif /* LAN8720_H */
