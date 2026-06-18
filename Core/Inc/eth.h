/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    eth.h
  * @brief   ETH peripheral configuration (RMII + LAN8720).
  ******************************************************************************
  */
/* USER CODE END Header */
#ifndef __ETH_H__
#define __ETH_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "stm32f4xx_hal_eth.h"

#include <stdbool.h>

extern ETH_HandleTypeDef heth;
extern ETH_DMADescTypeDef DMARxDscrTab[];
extern ETH_DMADescTypeDef DMATxDscrTab[];

bool MX_ETH_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __ETH_H__ */
