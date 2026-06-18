#include "can_app.h"

#include "can.h"
#include "can_if.h"
#include "j1939_app.h"
#include "uds_app.h"

#include "FreeRTOS.h"
#include "task.h"

#define CAN_BOOT_STACK 256U
#define CAN_BOOT_PRIO  (tskIDLE_PRIORITY + 2)

static void can_boot_task(void *argument)
{
  (void)argument;

  vTaskDelay(pdMS_TO_TICKS(50));
  CanIf_Init();
  J1939App_Init();
  UdsApp_Init();
  vTaskDelete(NULL);
}

void CanApp_Init(void)
{
  if (!MX_CAN1_Init() || !MX_CAN2_Init())
  {
    return;
  }

  (void)xTaskCreate(can_boot_task, "can_boot", CAN_BOOT_STACK, NULL, CAN_BOOT_PRIO, NULL);
}
