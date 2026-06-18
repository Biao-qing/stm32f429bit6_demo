/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    eth.c
  * @brief   ETH initialization (RMII + LAN8720).
  ******************************************************************************
  */
/* USER CODE END Header */

#include "eth.h"
#include "lan8720.h"

#include <stdbool.h>

ETH_HandleTypeDef heth;

ETH_DMADescTypeDef DMARxDscrTab[ETH_RX_DESC_CNT];
ETH_DMADescTypeDef DMATxDscrTab[ETH_TX_DESC_CNT];

static uint8_t s_eth_mac_addr[6] = {0x00U, 0x80U, 0xE1U, 0x00U, 0x00U, 0x00U};

bool MX_ETH_Init(void)
{
  heth.Instance = ETH;
  heth.Init.MACAddr = s_eth_mac_addr;
  heth.Init.MediaInterface = HAL_ETH_RMII_MODE;
  heth.Init.RxDesc = DMARxDscrTab;
  heth.Init.TxDesc = DMATxDscrTab;
  heth.Init.RxBuffLen = ETH_RX_BUF_SIZE;

  if (HAL_ETH_Init(&heth) != HAL_OK)
  {
    return false;
  }

  if (LAN8720_Init(&heth) != LAN8720_STATUS_OK)
  {
    return false;
  }

  return true;
}

void HAL_ETH_MspInit(ETH_HandleTypeDef *ethHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  if (ethHandle->Instance == ETH)
  {
    __HAL_RCC_ETH_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();

    /*
     * RMII + LAN8720 (野火 F429 常用引脚):
     * PA1  -> ETH_REF_CLK
     * PA2  -> ETH_MDIO
     * PA7  -> ETH_CRS_DV
     * PC1  -> ETH_MDC
     * PC4  -> ETH_RXD0
     * PC5  -> ETH_RXD1
     * PB11 -> ETH_TX_EN
     * PG13 -> ETH_TXD0
     * PG14 -> ETH_TXD1
     */
    GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_11;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_14;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(ETH_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(ETH_IRQn);
  }
}

void HAL_ETH_MspDeInit(ETH_HandleTypeDef *ethHandle)
{
  if (ethHandle->Instance == ETH)
  {
    __HAL_RCC_ETH_CLK_DISABLE();

    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7);
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_11);
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5);
    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_13 | GPIO_PIN_14);

    HAL_NVIC_DisableIRQ(ETH_IRQn);
  }
}
