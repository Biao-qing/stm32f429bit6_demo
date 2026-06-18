#ifndef CAN_USER_UDS_H
#define CAN_USER_UDS_H

#include "can_if.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  uint32_t rx_phy_id;
  uint32_t rx_func_id;
  uint32_t tx_id;
} CanUser_UdsConfig;

void CanUser_Uds_GetConfig(CanUser_UdsConfig *cfg);
void CanUser_Uds_NegativeResponse(uint8_t sid, uint8_t nrc, uint8_t *resp, uint32_t *resp_len);
int CanUser_Uds_HandleRequest(CanIfBus bus, const uint8_t *req, uint32_t req_len,
                              uint8_t *resp, uint32_t *resp_len);

#ifdef __cplusplus
}
#endif

#endif /* CAN_USER_UDS_H */
