#ifndef SYS_ARCH_H
#define SYS_ARCH_H

#include "lwip/opt.h"

#if (NO_SYS != 0)
#error "NO_SYS must be 0 for FreeRTOS port"
#endif

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "queue.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SYS_MBOX_NULL   ((sys_mbox_t)NULL)
#define SYS_SEM_NULL    ((sys_sem_t)NULL)

typedef SemaphoreHandle_t sys_sem_t;
typedef SemaphoreHandle_t sys_mutex_t;
typedef QueueHandle_t sys_mbox_t;
typedef TaskHandle_t sys_thread_t;

#ifdef __cplusplus
}
#endif

#endif /* SYS_ARCH_H */
