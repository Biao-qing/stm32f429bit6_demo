#ifndef CAN_H
#define CAN_H

#include "stm32f4xx_hal.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;

bool MX_CAN1_Init(void);
bool MX_CAN2_Init(void);

CAN_HandleTypeDef *CAN1_Handle(void);
CAN_HandleTypeDef *CAN2_Handle(void);

#ifdef __cplusplus
}
#endif

#endif /* CAN_H */
