#ifndef CAN_IF_H
#define CAN_IF_H

#include "stm32f4xx_hal.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  CAN_IF_BUS1 = 0,
  CAN_IF_BUS2 = 1,
  CAN_IF_BUS_COUNT = 2
} CanIfBus;

typedef struct {
  uint32_t id;
  uint8_t dlc;
  bool extended;
  uint8_t data[8];
} CanIfFrame;

void CanIf_Init(void);
void CanIf_OnRxPending(CAN_HandleTypeDef *hcan);

bool CanIf_Send(CanIfBus bus, uint32_t id, bool extended, const uint8_t data[], uint8_t dlc);
bool CanIf_RecvJ1939(CanIfBus bus, CanIfFrame *frame);
bool CanIf_RecvUds(CanIfBus bus, CanIfFrame *frame);

CAN_HandleTypeDef *CanIf_Handle(CanIfBus bus);

#ifdef __cplusplus
}
#endif

#endif /* CAN_IF_H */
