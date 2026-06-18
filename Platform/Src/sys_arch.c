#include "lwip/sys.h"
#include "lwip/opt.h"
#include "lwip/def.h"
#include "arch/sys_arch.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#if !NO_SYS

static sys_mutex_t s_lwip_protect_mutex;

void sys_init(void)
{
  s_lwip_protect_mutex = xSemaphoreCreateRecursiveMutex();
  LWIP_ASSERT("failed to create lwip protect mutex", s_lwip_protect_mutex != NULL);
}

u32_t sys_now(void)
{
  return (u32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
  *sem = xSemaphoreCreateCounting(UINT16_MAX, count);
  if (*sem == NULL)
  {
    return ERR_MEM;
  }

  return ERR_OK;
}

void sys_sem_free(sys_sem_t *sem)
{
  vSemaphoreDelete(*sem);
}

int sys_sem_valid(sys_sem_t *sem)
{
  return (*sem != SYS_SEM_NULL) ? 1 : 0;
}

void sys_sem_set_invalid(sys_sem_t *sem)
{
  *sem = SYS_SEM_NULL;
}

void sys_sem_signal(sys_sem_t *sem)
{
  xSemaphoreGive(*sem);
}

u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout_ms)
{
  TickType_t start = xTaskGetTickCount();
  TickType_t timeout_ticks = (timeout_ms == 0U) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);

  if (xSemaphoreTake(*sem, timeout_ticks) != pdTRUE)
  {
    return SYS_ARCH_TIMEOUT;
  }

  return (u32_t)((xTaskGetTickCount() - start) * portTICK_PERIOD_MS);
}

err_t sys_mutex_new(sys_mutex_t *mutex)
{
  *mutex = xSemaphoreCreateRecursiveMutex();
  if (*mutex == NULL)
  {
    return ERR_MEM;
  }

  return ERR_OK;
}

void sys_mutex_free(sys_mutex_t *mutex)
{
  vSemaphoreDelete(*mutex);
}

void sys_mutex_lock(sys_mutex_t *mutex)
{
  xSemaphoreTakeRecursive(*mutex, portMAX_DELAY);
}

void sys_mutex_unlock(sys_mutex_t *mutex)
{
  xSemaphoreGiveRecursive(*mutex);
}

err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
  *mbox = xQueueCreate((UBaseType_t)size, sizeof(void *));
  if (*mbox == NULL)
  {
    return ERR_MEM;
  }

  return ERR_OK;
}

void sys_mbox_free(sys_mbox_t *mbox)
{
  vQueueDelete(*mbox);
}

int sys_mbox_valid(sys_mbox_t *mbox)
{
  return (*mbox != SYS_MBOX_NULL) ? 1 : 0;
}

void sys_mbox_set_invalid(sys_mbox_t *mbox)
{
  *mbox = SYS_MBOX_NULL;
}

void sys_mbox_post(sys_mbox_t *mbox, void *msg)
{
  while (xQueueSend(*mbox, &msg, portMAX_DELAY) != pdTRUE)
  {
  }
}

err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
  if (xQueueSend(*mbox, &msg, 0) == pdTRUE)
  {
    return ERR_OK;
  }

  return ERR_MEM;
}

err_t sys_mbox_trypost_fromisr(sys_mbox_t *mbox, void *msg)
{
  BaseType_t woken = pdFALSE;

  if (xQueueSendFromISR(*mbox, &msg, &woken) != pdTRUE)
  {
    return ERR_MEM;
  }

  portYIELD_FROM_ISR(woken);
  return ERR_OK;
}

u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout_ms)
{
  TickType_t start = xTaskGetTickCount();
  TickType_t timeout_ticks = (timeout_ms == 0U) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
  void *dummy = NULL;

  if (msg == NULL)
  {
    msg = &dummy;
  }

  if (xQueueReceive(*mbox, msg, timeout_ticks) != pdTRUE)
  {
    return SYS_ARCH_TIMEOUT;
  }

  return (u32_t)((xTaskGetTickCount() - start) * portTICK_PERIOD_MS);
}

u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
  void *dummy = NULL;

  if (msg == NULL)
  {
    msg = &dummy;
  }

  if (xQueueReceive(*mbox, msg, 0) == pdTRUE)
  {
    return ERR_OK;
  }

  return SYS_MBOX_EMPTY;
}

sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread, void *arg,
                            int stacksize, int prio)
{
  TaskHandle_t handle = NULL;

  if (xTaskCreate(thread, name, (uint16_t)stacksize, arg, (UBaseType_t)prio, &handle) != pdPASS)
  {
    return NULL;
  }

  return handle;
}

sys_prot_t sys_arch_protect(void)
{
  xSemaphoreTakeRecursive(s_lwip_protect_mutex, portMAX_DELAY);
  return 0;
}

void sys_arch_unprotect(sys_prot_t pval)
{
  (void)pval;
  xSemaphoreGiveRecursive(s_lwip_protect_mutex);
}

#endif /* !NO_SYS */
