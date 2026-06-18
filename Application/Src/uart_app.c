#include "uart_app.h"

#include "flashdb_app.h"
#include "usart.h"

#include "FreeRTOS.h"
#include "task.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define UART_CMD_BUF_LEN 48U
#define UART_TASK_STACK  1024U
#define UART_TASK_PRIO   (tskIDLE_PRIORITY + 2)
#define UART_RX_POLL_MS  50U
#define UART_CMD_IDLE_MS 400U

static void uart_send(const char *msg)
{
  HAL_UART_Transmit(&huart1, (const uint8_t *)msg, (uint16_t)strlen(msg), HAL_MAX_DELAY);
}

static void uart_trim(char *cmd)
{
  size_t len;
  size_t start = 0U;

  if (cmd == NULL)
  {
    return;
  }

  while (cmd[start] != '\0' && isspace((unsigned char)cmd[start]))
  {
    start++;
  }

  if (start > 0U)
  {
    memmove(cmd, cmd + start, strlen(cmd + start) + 1U);
  }

  len = strlen(cmd);
  while (len > 0U && isspace((unsigned char)cmd[len - 1U]))
  {
    cmd[--len] = '\0';
  }
}

static void uart_send_help(void)
{
  uart_send("Commands (no Enter needed):\r\n");
  uart_send("  help       show help\r\n");
  uart_send("  log        dump logs (newest first)\r\n");
  uart_send("  log all    dump logs (oldest first)\r\n");
  uart_send("  log cnt    show log count\r\n");
}

static bool uart_log_wait_ready(void)
{
  if (FlashDB_App_IsReady())
  {
    return true;
  }

  if (FlashDB_App_GetState() == FLASHDB_STATE_FAILED)
  {
    uart_send("[log] FlashDB init failed\r\n");
    return false;
  }

  uart_send("[log] waiting for FlashDB...\r\n");
  if (!FlashDB_App_WaitReady(60000U))
  {
    if (FlashDB_App_GetState() == FLASHDB_STATE_INITING)
    {
      uart_send("[log] FlashDB still initializing, retry later\r\n");
    }
    else
    {
      uart_send("[log] FlashDB not ready\r\n");
    }
    return false;
  }

  return true;
}

static void uart_handle_command(char *cmd)
{
  uart_trim(cmd);

  if (cmd[0] == '\0')
  {
    return;
  }

  if ((strcmp(cmd, "help") == 0) || (strcmp(cmd, "?") == 0))
  {
    uart_send_help();
  }
  else if (strcmp(cmd, "log cnt") == 0)
  {
    char line[48];
    if (!uart_log_wait_ready())
    {
      return;
    }
    snprintf(line, sizeof(line), "[log] count=%u\r\n", (unsigned)FlashDB_App_LogCount());
    uart_send(line);
  }
  else if (strcmp(cmd, "log all") == 0)
  {
    if (!uart_log_wait_ready())
    {
      return;
    }
    uart_send("[log] dumping oldest first...\r\n");
    FlashDB_App_LogDump(false, 0U);
  }
  else if (strcmp(cmd, "log") == 0)
  {
    if (!uart_log_wait_ready())
    {
      return;
    }
    uart_send("[log] dumping newest first...\r\n");
    FlashDB_App_LogDump(true, 0U);
  }
  else
  {
    uart_send("Unknown command, type 'help'\r\n");
  }
}

static bool uart_is_complete_command(const char *cmd)
{
  return (strcmp(cmd, "help") == 0) || (strcmp(cmd, "?") == 0) ||
         (strcmp(cmd, "log all") == 0) || (strcmp(cmd, "log cnt") == 0);
}

static void uart_submit(char *cmd_buf, size_t *cmd_len)
{
  if (*cmd_len == 0U)
  {
    return;
  }

  uart_send("\r\n");
  cmd_buf[*cmd_len] = '\0';
  uart_handle_command(cmd_buf);
  *cmd_len = 0U;
  uart_send("> ");
}

static void uart_cmd_task(void *argument)
{
  char cmd_buf[UART_CMD_BUF_LEN];
  size_t cmd_len = 0U;
  uint8_t rx_byte;
  TickType_t last_rx_tick = 0U;

  (void)argument;

  uart_send("\r\nCLI ready. type 'help'\r\n> ");

  for (;;)
  {
    if (HAL_UART_Receive(&huart1, &rx_byte, 1, UART_RX_POLL_MS) == HAL_OK)
    {
      last_rx_tick = xTaskGetTickCount();

      if ((rx_byte == '\r') || (rx_byte == '\n'))
      {
        uart_submit(cmd_buf, &cmd_len);
        last_rx_tick = 0U;
      }
      else if ((rx_byte == '\b') || (rx_byte == 127U))
      {
        if (cmd_len > 0U)
        {
          cmd_len--;
          uart_send("\b \b");
        }
      }
      else if (cmd_len < (UART_CMD_BUF_LEN - 1U))
      {
        cmd_buf[cmd_len++] = (char)rx_byte;
        HAL_UART_Transmit(&huart1, &rx_byte, 1, HAL_MAX_DELAY);

        cmd_buf[cmd_len] = '\0';
        if (uart_is_complete_command(cmd_buf))
        {
          uart_submit(cmd_buf, &cmd_len);
          last_rx_tick = 0U;
        }
      }
    }
    else if ((cmd_len > 0U) && (last_rx_tick != 0U) &&
             ((xTaskGetTickCount() - last_rx_tick) >= pdMS_TO_TICKS(UART_CMD_IDLE_MS)))
    {
      uart_submit(cmd_buf, &cmd_len);
      last_rx_tick = 0U;
    }
    else
    {
      taskYIELD();
    }
  }
}

void UartApp_Init(void)
{
  xTaskCreate(uart_cmd_task, "uart_cmd", UART_TASK_STACK, NULL, UART_TASK_PRIO, NULL);
}
