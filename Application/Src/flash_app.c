#include "flash_app.h"

#include "flashdb_app.h"

#include "FreeRTOS.h"
#include "task.h"

#define FLASH_INIT_TASK_STACK 1536U
#define FLASH_INIT_TASK_PRIO  (tskIDLE_PRIORITY + 1)

static void flash_init_task(void *argument)
{
  (void)argument;
  FlashDB_App_Init();
  vTaskDelete(NULL);
}

void FlashApp_Init(void)
{
  BaseType_t ok;

  ok = xTaskCreate(flash_init_task, "flash_init", FLASH_INIT_TASK_STACK, NULL,
                   FLASH_INIT_TASK_PRIO, NULL);
  if (ok != pdPASS)
  {
    /* Heap full: skip async init; CLI 'log' will report not ready. */
    return;
  }
}
