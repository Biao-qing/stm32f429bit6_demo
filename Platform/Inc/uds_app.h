#ifndef UDS_APP_H
#define UDS_APP_H

/*
 * UDS 用户手写: Platform/Src/can_user_uds.c（见 can_user_uds.h）
 */

#include "can_if.h"

#ifdef __cplusplus
extern "C" {
#endif

void UdsApp_Init(void);
void UdsApp_Poll(CanIfBus bus);

#ifdef __cplusplus
}
#endif

#endif /* UDS_APP_H */
