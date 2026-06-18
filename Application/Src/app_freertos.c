#include "app_freertos.h"
#include "fdb_port.h"
#include "flash_app.h"
#include "led_app.h"
#include "net_app.h"
#include "can_app.h"
#include "uart_app.h"

#include "FreeRTOS.h"
#include "task.h"

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
  (void)xTask;
  (void)pcTaskName;
  taskDISABLE_INTERRUPTS();
  for (;;)
  {
  }
}

void vApplicationMallocFailedHook(void)
{
  taskDISABLE_INTERRUPTS();
  for (;;)
  {
  }
}

void App_FreeRTOS_Init(void)
{
  fdb_port_mutex_init();
  LedApp_Init();
  UartApp_Init();
  FlashApp_Init();
  CanApp_Init();
  NetApp_Init();
  vTaskStartScheduler();
}
