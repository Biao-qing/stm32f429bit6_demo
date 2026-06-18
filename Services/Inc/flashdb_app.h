#ifndef FLASHDB_APP_H
#define FLASHDB_APP_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum
{
  FLASHDB_STATE_IDLE = 0,
  FLASHDB_STATE_INITING,
  FLASHDB_STATE_READY,
  FLASHDB_STATE_FAILED,
} flashdb_state_t;

bool FlashDB_App_Init(void);
bool FlashDB_App_IsReady(void);
flashdb_state_t FlashDB_App_GetState(void);
bool FlashDB_App_WaitReady(uint32_t timeout_ms);
bool FlashDB_App_LogWrite(const char *msg);
size_t FlashDB_App_LogCount(void);
void FlashDB_App_LogDump(bool reverse, uint32_t max_count);
bool FlashDB_App_KvSetString(const char *key, const char *value);
const char *FlashDB_App_KvGetString(const char *key);

#endif /* FLASHDB_APP_H */
