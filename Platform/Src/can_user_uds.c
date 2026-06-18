#include "can_user_uds.h"

#include <string.h>

void CanUser_Uds_GetConfig(CanUser_UdsConfig *cfg)
{
  if (cfg == NULL)
  {
    return;
  }

  cfg->rx_phy_id = 0x7E0U;
  cfg->rx_func_id = 0x7DFU;
  cfg->tx_id = 0x7E8U;
}

void CanUser_Uds_NegativeResponse(uint8_t sid, uint8_t nrc, uint8_t *resp, uint32_t *resp_len)
{
  resp[0] = 0x7FU;
  resp[1] = sid;
  resp[2] = nrc;
  *resp_len = 3U;
}

int CanUser_Uds_HandleRequest(CanIfBus bus, const uint8_t *req, uint32_t req_len,
                              uint8_t *resp, uint32_t *resp_len)
{
  (void)bus;

  if (req == NULL || resp == NULL || resp_len == NULL || req_len == 0U)
  {
    return -1;
  }

  switch (req[0])
  {
  case 0x3EU:
    if (req_len >= 2U && req[1] == 0x00U)
    {
      resp[0] = 0x7EU;
      resp[1] = 0x00U;
      *resp_len = 2U;
      return 0;
    }
    break;

  case 0x10U:
    if (req_len >= 2U)
    {
      resp[0] = 0x50U;
      resp[1] = req[1];
      resp[2] = 0x00U;
      resp[3] = 0x32U;
      resp[4] = 0x01U;
      resp[5] = 0xF4U;
      *resp_len = 6U;
      return 0;
    }
    break;

  case 0x22U:
    if (req_len >= 3U)
    {
      uint16_t did = (uint16_t)(((uint16_t)req[1] << 8) | req[2]);

      if (did == 0xF190U)
      {
        resp[0] = 0x62U;
        resp[1] = req[1];
        resp[2] = req[2];
        memcpy(&resp[3], "DEMOVIN123456789", 17U);
        *resp_len = 20U;
        return 0;
      }
      if (did == 0x0100U)
      {
        resp[0] = 0x62U;
        resp[1] = req[1];
        resp[2] = req[2];
        resp[3] = 0x42U;
        *resp_len = 4U;
        return 0;
      }

      CanUser_Uds_NegativeResponse(0x22U, 0x31U, resp, resp_len);
      return 0;
    }
    break;

  default:
    break;
  }

  CanUser_Uds_NegativeResponse(req[0], 0x11U, resp, resp_len);
  return 0;
}
