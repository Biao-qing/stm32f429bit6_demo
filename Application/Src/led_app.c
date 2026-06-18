#include "led_app.h"

#include "main.h"

#include "FreeRTOS.h"
#include "task.h"

#define LED_TASK_STACK 256U
#define LED_TASK_PRIO  (tskIDLE_PRIORITY + 2)

#define LED1_BLINK_MS 500U  /* PD12: 1 Hz */
#define LED2_BLINK_MS 125U  /* PG7:  4 Hz */

static void led1_task(void *argument)
{
  (void)argument;

  for (;;)
  {
    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
    vTaskDelay(pdMS_TO_TICKS(LED1_BLINK_MS));
  }
}

static void led2_task(void *argument)
{
  (void)argument;

  for (;;)
  {
    HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
    vTaskDelay(pdMS_TO_TICKS(LED2_BLINK_MS));
  }
}

void LedApp_Init(void)
{
  BaseType_t ok;

  ok = xTaskCreate(led1_task, "led1", LED_TASK_STACK, NULL, LED_TASK_PRIO, NULL);
  configASSERT(ok == pdPASS);
  ok = xTaskCreate(led2_task, "led2", LED_TASK_STACK, NULL, LED_TASK_PRIO, NULL);
  configASSERT(ok == pdPASS);
}
