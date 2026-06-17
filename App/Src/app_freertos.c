#include "app_freertos.h"
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
  UartApp_Init();
  vTaskStartScheduler();
}
