#include "flashdb_app.h"

#include "fdb_port.h"
#include "usart.h"
#include "w25q64.h"

#include "FreeRTOS.h"
#include "task.h"

#include <flashdb.h>
#include <fal.h>

#include <limits.h>
#include <stdio.h>
#include <string.h>

#define FDB_LOG_TAG "[flashdb]"

#define TSDB_NAME "log"
#define TSDB_PART "fdb_tsdb"
#define KVDB_NAME "cfg"
#define KVDB_PART "fdb_kvdb"
#define TSDB_MAX_LEN 128U

static struct fdb_tsdb tsdb;
static struct fdb_kvdb kvdb;
static bool flashdb_ready;
static volatile flashdb_state_t flashdb_state = FLASHDB_STATE_IDLE;

static uint32_t boot_count;

static struct fdb_default_kv_node default_kv_table[] = {
    {"boot_count", &boot_count, sizeof(boot_count)},
};

static void uart_msg(const char *msg)
{
  HAL_UART_Transmit(&huart1, (const uint8_t *)msg, (uint16_t)strlen(msg), HAL_MAX_DELAY);
}

static void flashdb_mark_failed(void)
{
  flashdb_ready = false;
  flashdb_state = FLASHDB_STATE_FAILED;
}

bool FlashDB_App_IsReady(void)
{
  return flashdb_ready;
}

flashdb_state_t FlashDB_App_GetState(void)
{
  return flashdb_state;
}

bool FlashDB_App_WaitReady(uint32_t timeout_ms)
{
  TickType_t start = xTaskGetTickCount();

  while (!flashdb_ready)
  {
    if (flashdb_state == FLASHDB_STATE_FAILED)
    {
      return false;
    }

    if ((xTaskGetTickCount() - start) >= pdMS_TO_TICKS(timeout_ms))
    {
      return false;
    }

    vTaskDelay(pdMS_TO_TICKS(50));
  }

  return true;
}

bool FlashDB_App_Init(void)
{
  fdb_err_t result;
  struct fdb_default_kv default_kv;
  char line[96];
  uint8_t id[3];

  flashdb_state = FLASHDB_STATE_INITING;
  flashdb_ready = false;
  uart_msg("[flashdb] initializing...\r\n");

  if (!W25Q64_Init() || !W25Q64_ReadJedecId(id))
  {
    uart_msg("[flashdb] W25Q64 not ready\r\n");
    flashdb_mark_failed();
    return false;
  }

  snprintf(line, sizeof(line), "[flashdb] W25Q64 ID: %02X %02X %02X\r\n", id[0], id[1], id[2]);
  uart_msg(line);

  if (fal_init() <= 0)
  {
    uart_msg("[flashdb] FAL init failed\r\n");
    flashdb_mark_failed();
    return false;
  }

  default_kv.kvs = default_kv_table;
  default_kv.num = sizeof(default_kv_table) / sizeof(default_kv_table[0]);

  fdb_kvdb_control(&kvdb, FDB_KVDB_CTRL_SET_LOCK, (void *)fdb_port_lock);
  fdb_kvdb_control(&kvdb, FDB_KVDB_CTRL_SET_UNLOCK, (void *)fdb_port_unlock);
  result = fdb_kvdb_init(&kvdb, KVDB_NAME, KVDB_PART, &default_kv, NULL);
  if (result != FDB_NO_ERR)
  {
    uart_msg("[flashdb] KVDB init failed\r\n");
    flashdb_mark_failed();
    return false;
  }

  fdb_tsdb_control(&tsdb, FDB_TSDB_CTRL_SET_LOCK, (void *)fdb_port_lock);
  fdb_tsdb_control(&tsdb, FDB_TSDB_CTRL_SET_UNLOCK, (void *)fdb_port_unlock);
  result = fdb_tsdb_init(&tsdb, TSDB_NAME, TSDB_PART, fdb_port_get_time, TSDB_MAX_LEN, NULL);
  if (result != FDB_NO_ERR)
  {
    uart_msg("[flashdb] TSDB init failed\r\n");
    flashdb_mark_failed();
    return false;
  }

  fdb_tsdb_control(&tsdb, FDB_TSDB_CTRL_SET_ROLLOVER, (void *)1);

  {
    struct fdb_blob blob;
    if (fdb_kv_get_blob(&kvdb, "boot_count", fdb_blob_make(&blob, &boot_count, sizeof(boot_count))) == 0U)
    {
      boot_count = 0U;
    }
    boot_count++;
    fdb_kv_set_blob(&kvdb, "boot_count", fdb_blob_make(&blob, &boot_count, sizeof(boot_count)));
  }

  flashdb_ready = true;
  flashdb_state = FLASHDB_STATE_READY;
  snprintf(line, sizeof(line), "[flashdb] init ok, boot_count=%lu\r\n", (unsigned long)boot_count);
  uart_msg(line);
  FlashDB_App_LogWrite("FlashDB initialized");
  return true;
}

bool FlashDB_App_LogWrite(const char *msg)
{
  struct fdb_blob blob;

  if (!flashdb_ready || (msg == NULL))
  {
    return false;
  }

  return fdb_tsl_append(&tsdb, fdb_blob_make(&blob, msg, strlen(msg) + 1U)) == FDB_NO_ERR;
}

bool FlashDB_App_KvSetString(const char *key, const char *value)
{
  if (!flashdb_ready || (key == NULL) || (value == NULL))
  {
    return false;
  }

  return fdb_kv_set(&kvdb, key, value) == FDB_NO_ERR;
}

const char *FlashDB_App_KvGetString(const char *key)
{
  if (!flashdb_ready || (key == NULL))
  {
    return NULL;
  }

  return fdb_kv_get(&kvdb, key);
}

typedef struct
{
  uint32_t printed;
  uint32_t max_count;
} log_dump_ctx_t;

static bool log_dump_cb(fdb_tsl_t tsl, void *arg)
{
  log_dump_ctx_t *ctx = arg;
  struct fdb_blob blob;
  char buf[TSDB_MAX_LEN];
  char line[160];

  memset(buf, 0, sizeof(buf));
  fdb_blob_read((fdb_db_t)&tsdb, fdb_tsl_to_blob(tsl, fdb_blob_make(&blob, buf, sizeof(buf))));
  snprintf(line, sizeof(line), "[log] t=%ld: %s\r\n", (long)tsl->time, buf);
  uart_msg(line);

  ctx->printed++;
  if ((ctx->max_count > 0U) && (ctx->printed >= ctx->max_count))
  {
    return true;
  }

  return false;
}

size_t FlashDB_App_LogCount(void)
{
  if (!flashdb_ready)
  {
    return 0U;
  }

  return fdb_tsl_query_count(&tsdb, 0, (fdb_time_t)INT32_MAX, FDB_TSL_WRITE);
}

void FlashDB_App_LogDump(bool reverse, uint32_t max_count)
{
  log_dump_ctx_t ctx = {.printed = 0U, .max_count = max_count};
  char line[64];

  if (!flashdb_ready)
  {
    if (flashdb_state == FLASHDB_STATE_INITING)
    {
      uart_msg("[log] FlashDB still initializing\r\n");
    }
    else if (flashdb_state == FLASHDB_STATE_FAILED)
    {
      uart_msg("[log] FlashDB init failed\r\n");
    }
    else
    {
      uart_msg("[log] FlashDB not ready\r\n");
    }
    return;
  }

  snprintf(line, sizeof(line), "[log] total=%u, showing %s%s\r\n",
           (unsigned)FlashDB_App_LogCount(),
           reverse ? "newest" : "oldest",
           (max_count > 0U) ? " (limited)" : "");
  uart_msg(line);

  if (reverse)
  {
    fdb_tsl_iter_reverse(&tsdb, log_dump_cb, &ctx);
  }
  else
  {
    fdb_tsl_iter(&tsdb, log_dump_cb, &ctx);
  }

  snprintf(line, sizeof(line), "[log] done, printed=%lu\r\n", (unsigned long)ctx.printed);
  uart_msg(line);
}
