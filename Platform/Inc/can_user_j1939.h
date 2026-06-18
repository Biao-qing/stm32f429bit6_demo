#ifndef CAN_USER_J1939_H
#define CAN_USER_J1939_H

/*
 * J1939 用户手写:
 *   can_user_j1939.c      — 3 个回调函数
 *   can_user_j1939_msg.h  — Message / Signal 矩阵（DBC 风格）
 */

#include "can_if.h"

#include "Open_SAE_J1939/Open_SAE_J1939.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void CanUser_J1939_FillEcu(CanIfBus bus, J1939 *j1939);
void CanUser_J1939_OnPgnRx(CanIfBus bus, uint32_t pgn, uint8_t sa, const uint8_t data[8]);
void CanUser_J1939_Periodic(CanIfBus bus, uint32_t tick_ms, J1939 *j1939);

uint32_t CanUser_J1939_IdToPgn(uint32_t id);
bool CanUser_J1939_SendRaw(CanIfBus bus, uint32_t id, const uint8_t data[8]);

#ifdef __cplusplus
}
#endif

#endif /* CAN_USER_J1939_H */
