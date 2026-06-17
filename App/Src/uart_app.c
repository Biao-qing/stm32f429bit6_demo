#include "uart_app.h"

#include "usart.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include <string.h>

#define UART_RX_QUEUE_LEN 32U
#define UART_TASK_STACK   256U
#define UART_TASK_PRIO    (tskIDLE_PRIORITY + 2)

static QueueHandle_t uart_rx_queue;
static uint8_t uart_rx_byte;

static void uart_send(const char *msg)
{
  HAL_UART_Transmit(&huart1, (const uint8_t *)msg, (uint16_t)strlen(msg), HAL_MAX_DELAY);
}

static void uart_echo_task(void *argument)
{
  uint8_t rx_byte;

  (void)argument;

  uart_send("\r\n=== USART1 FreeRTOS Test ===\r\n");
  uart_send("Baud: 115200 8N1, PA9=TX PA10=RX\r\n");
  uart_send("Type anything, MCU echoes back.\r\n\r\n");

  HAL_UART_Receive_IT(&huart1, &uart_rx_byte, 1);

  for (;;)
  {
    if (xQueueReceive(uart_rx_queue, &rx_byte, portMAX_DELAY) == pdTRUE)
    {
      HAL_UART_Transmit(&huart1, &rx_byte, 1, 100);
    }
  }
}

static void uart_heartbeat_task(void *argument)
{
  (void)argument;

  for (;;)
  {
    vTaskDelay(pdMS_TO_TICKS(5000));
    uart_send("[heartbeat]\r\n");
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  if (huart->Instance == USART1)
  {
    xQueueSendFromISR(uart_rx_queue, &uart_rx_byte, &xHigherPriorityTaskWoken);
    HAL_UART_Receive_IT(&huart1, &uart_rx_byte, 1);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
}

void UartApp_Init(void)
{
  uart_rx_queue = xQueueCreate(UART_RX_QUEUE_LEN, sizeof(uint8_t));
  configASSERT(uart_rx_queue != NULL);

  xTaskCreate(uart_echo_task, "uart_echo", UART_TASK_STACK, NULL, UART_TASK_PRIO, NULL);
  xTaskCreate(uart_heartbeat_task, "uart_hb", UART_TASK_STACK, NULL, UART_TASK_PRIO - 1, NULL);
}
