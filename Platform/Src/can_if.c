#include "can_if.h"

#include "can.h"

#include "FreeRTOS.h"
#include "semphr.h"

#include <string.h>

#define CAN_IF_RX_DEPTH 32U
#define CAN_IF_UDS_PHY_RX_ID   0x7E0U
#define CAN_IF_UDS_FUNC_RX_ID  0x7DFU

typedef struct {
  CanIfFrame frames[CAN_IF_RX_DEPTH];
  volatile uint16_t head;
  volatile uint16_t tail;
  SemaphoreHandle_t mutex;
} CanIfQueue;

static CanIfQueue s_j1939_q[CAN_IF_BUS_COUNT];
static CanIfQueue s_uds_q[CAN_IF_BUS_COUNT];

static bool can_if_queue_push(CanIfQueue *q, const CanIfFrame *frame)
{
  if (q == NULL || frame == NULL)
  {
    return false;
  }

  if (xSemaphoreTake(q->mutex, pdMS_TO_TICKS(2)) != pdTRUE)
  {
    return false;
  }

  uint16_t next = (uint16_t)((q->head + 1U) % CAN_IF_RX_DEPTH);
  if (next == q->tail)
  {
    xSemaphoreGive(q->mutex);
    return false;
  }

  q->frames[q->head] = *frame;
  q->head = next;
  xSemaphoreGive(q->mutex);
  return true;
}

static bool can_if_queue_pop(CanIfQueue *q, CanIfFrame *frame)
{
  if (q == NULL || frame == NULL)
  {
    return false;
  }

  if (xSemaphoreTake(q->mutex, pdMS_TO_TICKS(2)) != pdTRUE)
  {
    return false;
  }

  if (q->tail == q->head)
  {
    xSemaphoreGive(q->mutex);
    return false;
  }

  *frame = q->frames[q->tail];
  q->tail = (uint16_t)((q->tail + 1U) % CAN_IF_RX_DEPTH);
  xSemaphoreGive(q->mutex);
  return true;
}

static void can_if_route_rx(CanIfBus bus, const CanIfFrame *frame)
{
  if (frame->extended)
  {
    (void)can_if_queue_push(&s_j1939_q[bus], frame);
    return;
  }

  if (frame->id == CAN_IF_UDS_PHY_RX_ID || frame->id == CAN_IF_UDS_FUNC_RX_ID)
  {
    (void)can_if_queue_push(&s_uds_q[bus], frame);
  }
}

bool CanIf_RecvJ1939(CanIfBus bus, CanIfFrame *frame)
{
  if (bus >= CAN_IF_BUS_COUNT)
  {
    return false;
  }

  return can_if_queue_pop(&s_j1939_q[bus], frame);
}

bool CanIf_RecvUds(CanIfBus bus, CanIfFrame *frame)
{
  if (bus >= CAN_IF_BUS_COUNT)
  {
    return false;
  }

  return can_if_queue_pop(&s_uds_q[bus], frame);
}

bool CanIf_Send(CanIfBus bus, uint32_t id, bool extended, const uint8_t data[], uint8_t dlc)
{
  CAN_HandleTypeDef *hcan = CanIf_Handle(bus);
  CAN_TxHeaderTypeDef tx = {0};
  uint32_t mailbox;

  if (hcan == NULL || data == NULL || dlc > 8U || bus >= CAN_IF_BUS_COUNT)
  {
    return false;
  }

  tx.DLC = dlc;
  tx.RTR = CAN_RTR_DATA;
  tx.IDE = extended ? CAN_ID_EXT : CAN_ID_STD;
  tx.TransmitGlobalTime = DISABLE;
  if (extended)
  {
    tx.ExtId = id;
    tx.StdId = 0U;
  }
  else
  {
    tx.StdId = (uint32_t)(id & 0x7FFU);
    tx.ExtId = 0U;
  }

  return HAL_CAN_AddTxMessage(hcan, &tx, (uint8_t *)data, &mailbox) == HAL_OK;
}

CAN_HandleTypeDef *CanIf_Handle(CanIfBus bus)
{
  if (bus == CAN_IF_BUS1)
  {
    return CAN1_Handle();
  }
  if (bus == CAN_IF_BUS2)
  {
    return CAN2_Handle();
  }
  return NULL;
}

void CanIf_Init(void)
{
  for (uint32_t i = 0U; i < CAN_IF_BUS_COUNT; i++)
  {
    s_j1939_q[i].head = 0U;
    s_j1939_q[i].tail = 0U;
    s_j1939_q[i].mutex = xSemaphoreCreateMutex();

    s_uds_q[i].head = 0U;
    s_uds_q[i].tail = 0U;
    s_uds_q[i].mutex = xSemaphoreCreateMutex();
  }
}

void CanIf_OnRxPending(CAN_HandleTypeDef *hcan)
{
  CanIfBus bus;
  CAN_RxHeaderTypeDef hdr = {0};
  CanIfFrame frame = {0};

  if (hcan == NULL)
  {
    return;
  }

  if (hcan->Instance == CAN1)
  {
    bus = CAN_IF_BUS1;
  }
  else if (hcan->Instance == CAN2)
  {
    bus = CAN_IF_BUS2;
  }
  else
  {
    return;
  }

  while (HAL_CAN_GetRxFifoFillLevel(hcan, CAN_RX_FIFO0) > 0U)
  {
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &hdr, frame.data) != HAL_OK)
    {
      break;
    }

    frame.extended = (hdr.IDE == CAN_ID_EXT);
    frame.id = frame.extended ? hdr.ExtId : hdr.StdId;
    frame.dlc = hdr.DLC;
    can_if_route_rx(bus, &frame);
  }
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  CanIf_OnRxPending(hcan);
}
