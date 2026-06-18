#include "lan8720.h"
#include "main.h"

#define LAN8720_RESET_HOLD_MS   10U
#define LAN8720_RESET_RELEASE_MS 50U

static void lan8720_delay_ms(uint32_t ms)
{
  uint32_t start = HAL_GetTick();

  while ((HAL_GetTick() - start) < ms)
  {
  }
}

static LAN8720_StatusTypeDef lan8720_read_reg(ETH_HandleTypeDef *heth,
                                              uint32_t reg,
                                              uint32_t *value)
{
  if (HAL_ETH_ReadPHYRegister(heth, LAN8720_PHY_ADDRESS, reg, value) != HAL_OK)
  {
    return LAN8720_STATUS_ERROR;
  }

  return LAN8720_STATUS_OK;
}

static LAN8720_StatusTypeDef lan8720_write_reg(ETH_HandleTypeDef *heth,
                                               uint32_t reg,
                                               uint32_t value)
{
  if (HAL_ETH_WritePHYRegister(heth, LAN8720_PHY_ADDRESS, reg, value) != HAL_OK)
  {
    return LAN8720_STATUS_ERROR;
  }

  return LAN8720_STATUS_OK;
}

void LAN8720_Reset(void)
{
  HAL_GPIO_WritePin(ETH_RESET_GPIO_Port, ETH_RESET_Pin, GPIO_PIN_RESET);
  lan8720_delay_ms(LAN8720_RESET_HOLD_MS);
  HAL_GPIO_WritePin(ETH_RESET_GPIO_Port, ETH_RESET_Pin, GPIO_PIN_SET);
  lan8720_delay_ms(LAN8720_RESET_RELEASE_MS);
}

LAN8720_StatusTypeDef LAN8720_Init(ETH_HandleTypeDef *heth)
{
  uint32_t reg_value = 0U;

  LAN8720_Reset();

  if (lan8720_read_reg(heth, LAN8720_BCR, &reg_value) != LAN8720_STATUS_OK)
  {
    return LAN8720_STATUS_ERROR;
  }

  reg_value |= LAN8720_BCR_SOFT_RESET;
  if (lan8720_write_reg(heth, LAN8720_BCR, reg_value) != LAN8720_STATUS_OK)
  {
    return LAN8720_STATUS_ERROR;
  }

  do
  {
    if (lan8720_read_reg(heth, LAN8720_BCR, &reg_value) != LAN8720_STATUS_OK)
    {
      return LAN8720_STATUS_ERROR;
    }
  } while ((reg_value & LAN8720_BCR_SOFT_RESET) != 0U);

  HAL_ETH_SetMDIOClockRange(heth);

  return LAN8720_STATUS_OK;
}

LAN8720_LinkStateTypeDef LAN8720_GetLinkState(ETH_HandleTypeDef *heth)
{
  uint32_t bsr = 0U;

  if (lan8720_read_reg(heth, LAN8720_BSR, &bsr) != LAN8720_STATUS_OK)
  {
    return LAN8720_LINK_DOWN;
  }

  if ((bsr & LAN8720_BSR_LINK_STATUS) != 0U)
  {
    return LAN8720_LINK_UP;
  }

  return LAN8720_LINK_DOWN;
}

LAN8720_PhyStateTypeDef LAN8720_GetPhyState(ETH_HandleTypeDef *heth)
{
  uint32_t bsr = 0U;
  uint32_t physcsr = 0U;

  if (lan8720_read_reg(heth, LAN8720_BSR, &bsr) != LAN8720_STATUS_OK)
  {
    return LAN8720_PHY_LINK_DOWN;
  }

  if ((bsr & LAN8720_BSR_LINK_STATUS) == 0U)
  {
    return LAN8720_PHY_LINK_DOWN;
  }

  if (lan8720_read_reg(heth, LAN8720_PHYSCSR, &physcsr) != LAN8720_STATUS_OK)
  {
    return LAN8720_PHY_100M_FULL;
  }

  if ((physcsr & LAN8720_SPEED_STATUS) != 0U)
  {
    if ((physcsr & LAN8720_DUPLEX_STATUS) != 0U)
    {
      return LAN8720_PHY_100M_FULL;
    }

    return LAN8720_PHY_100M_HALF;
  }

  if ((physcsr & LAN8720_DUPLEX_STATUS) != 0U)
  {
    return LAN8720_PHY_10M_FULL;
  }

  return LAN8720_PHY_10M_HALF;
}
