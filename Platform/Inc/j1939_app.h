#ifndef J1939_APP_H
#define J1939_APP_H

/*
 * J1939 用户手写: Platform/Src/can_user_j1939.c（见 can_user_j1939.h）
 */

#include "can_if.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void J1939App_Init(void);
bool J1939App_ProcessOneFrame(CanIfBus bus);

#ifdef __cplusplus
}
#endif

#endif /* J1939_APP_H */
