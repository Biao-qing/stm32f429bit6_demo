/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.c
  * @brief   CAN1/CAN2 initialization (STM32F429 has two bxCAN controllers).
  ******************************************************************************
  */
/* USER CODE END Header */

#include "can.h"

CAN_HandleTypeDef hcan1;
CAN_HandleTypeDef hcan2;

/* CAN2 shares filter banks with CAN1; split at bank 14. */
#define CAN_SLAVE_START_FILTER_BANK 14U

/*
 * 野火 F429 挑战者常用引脚（与板载 TJA1050 一致）:
 * CAN1: PB8(RX) PB9(TX)
 * CAN2: PB12(RX) PB13(TX)
 *
 * APB1 CAN clock = 45 MHz -> 500 kbit/s (Prescaler=9, BS1=7, BS2=2).
 */
static void can_apply_bit_timing(CAN_HandleTypeDef *hcan)
{
  hcan->Init.TimeTriggeredMode = DISABLE;
  hcan->Init.AutoBusOff = ENABLE;
  hcan->Init.AutoWakeUp = DISABLE;
  hcan->Init.AutoRetransmission = ENABLE;
  hcan->Init.ReceiveFifoLocked = DISABLE;
  hcan->Init.TransmitFifoPriority = DISABLE;
  hcan->Init.Mode = CAN_MODE_NORMAL;
  hcan->Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan->Init.TimeSeg1 = CAN_BS1_7TQ;
  hcan->Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan->Init.Prescaler = 9U;
}

static bool can_config_accept_all_filter(CAN_HandleTypeDef *hcan, uint32_t filter_bank,
                                         bool set_slave_start)
{
  CAN_FilterTypeDef filter = {0};

  filter.FilterBank = filter_bank;
  filter.FilterMode = CAN_FILTERMODE_IDMASK;
  filter.FilterScale = CAN_FILTERSCALE_32BIT;
  filter.FilterIdHigh = 0U;
  filter.FilterIdLow = 0U;
  filter.FilterMaskIdHigh = 0U;
  filter.FilterMaskIdLow = 0U;
  filter.FilterFIFOAssignment = CAN_RX_FIFO0;
  filter.FilterActivation = ENABLE;
  if (set_slave_start)
  {
    filter.SlaveStartFilterBank = CAN_SLAVE_START_FILTER_BANK;
  }

  return HAL_CAN_ConfigFilter(hcan, &filter) == HAL_OK;
}

static bool can_start_controller(CAN_HandleTypeDef *hcan)
{
  if (HAL_CAN_Start(hcan) != HAL_OK)
  {
    return false;
  }

  return HAL_CAN_ActivateNotification(hcan,
                                    CAN_IT_RX_FIFO0_MSG_PENDING |
                                        CAN_IT_ERROR_WARNING |
                                        CAN_IT_ERROR_PASSIVE |
                                        CAN_IT_BUSOFF) == HAL_OK;
}

bool MX_CAN1_Init(void)
{
  hcan1.Instance = CAN1;
  can_apply_bit_timing(&hcan1);

  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    return false;
  }

  if (!can_config_accept_all_filter(&hcan1, 0U, true))
  {
    return false;
  }

  return can_start_controller(&hcan1);
}

bool MX_CAN2_Init(void)
{
  hcan2.Instance = CAN2;
  can_apply_bit_timing(&hcan2);

  if (HAL_CAN_Init(&hcan2) != HAL_OK)
  {
    return false;
  }

  if (!can_config_accept_all_filter(&hcan2, CAN_SLAVE_START_FILTER_BANK, false))
  {
    return false;
  }

  return can_start_controller(&hcan2);
}

CAN_HandleTypeDef *CAN1_Handle(void)
{
  return &hcan1;
}

CAN_HandleTypeDef *CAN2_Handle(void)
{
  return &hcan2;
}

void HAL_CAN_MspInit(CAN_HandleTypeDef *canHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  if (canHandle->Instance == CAN1)
  {
    __HAL_RCC_CAN1_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(CAN1_TX_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(CAN1_TX_IRQn);
    HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
    HAL_NVIC_SetPriority(CAN1_RX1_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX1_IRQn);
    HAL_NVIC_SetPriority(CAN1_SCE_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(CAN1_SCE_IRQn);
  }
  else if (canHandle->Instance == CAN2)
  {
    __HAL_RCC_CAN1_CLK_ENABLE();
    __HAL_RCC_CAN2_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(CAN2_TX_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(CAN2_TX_IRQn);
    HAL_NVIC_SetPriority(CAN2_RX0_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(CAN2_RX0_IRQn);
    HAL_NVIC_SetPriority(CAN2_RX1_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(CAN2_RX1_IRQn);
    HAL_NVIC_SetPriority(CAN2_SCE_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(CAN2_SCE_IRQn);
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef *canHandle)
{
  if (canHandle->Instance == CAN1)
  {
    __HAL_RCC_CAN1_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8 | GPIO_PIN_9);

    HAL_NVIC_DisableIRQ(CAN1_TX_IRQn);
    HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
    HAL_NVIC_DisableIRQ(CAN1_RX1_IRQn);
    HAL_NVIC_DisableIRQ(CAN1_SCE_IRQn);
  }
  else if (canHandle->Instance == CAN2)
  {
    __HAL_RCC_CAN2_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12 | GPIO_PIN_13);

    HAL_NVIC_DisableIRQ(CAN2_TX_IRQn);
    HAL_NVIC_DisableIRQ(CAN2_RX0_IRQn);
    HAL_NVIC_DisableIRQ(CAN2_RX1_IRQn);
    HAL_NVIC_DisableIRQ(CAN2_SCE_IRQn);
  }
}
