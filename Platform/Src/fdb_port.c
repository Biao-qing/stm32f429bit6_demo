#include "fdb_port.h"

#include "usart.h"

#include "FreeRTOS.h"
#include "semphr.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static SemaphoreHandle_t fdb_mutex;

void fdb_port_print(const char *fmt, ...)
{
  char buf[160];
  va_list args;

  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  HAL_UART_Transmit(&huart1, (uint8_t *)buf, (uint16_t)strlen(buf), HAL_MAX_DELAY);
}

void fdb_port_lock(fdb_db_t db)
{
  (void)db;

  if (fdb_mutex != NULL && xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
  {
    xSemaphoreTake(fdb_mutex, portMAX_DELAY);
  }
}

void fdb_port_unlock(fdb_db_t db)
{
  (void)db;

  if (fdb_mutex != NULL && xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
  {
    xSemaphoreGive(fdb_mutex);
  }
}

void fdb_port_mutex_init(void)
{
  if (fdb_mutex == NULL)
  {
    fdb_mutex = xSemaphoreCreateMutex();
    configASSERT(fdb_mutex != NULL);
  }
}

fdb_time_t fdb_port_get_time(void)
{
  return (fdb_time_t)(HAL_GetTick() / 1000U);
}
