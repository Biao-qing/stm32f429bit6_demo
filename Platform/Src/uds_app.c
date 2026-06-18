#include "uds_app.h"

#include "can_if.h"
#include "can_user_uds.h"

#include "isotp.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdint.h>

#define UDS_ISOTP_BUF_SIZE 256U

typedef struct {
  IsoTpLink link;
  uint8_t tx_buf[UDS_ISOTP_BUF_SIZE];
  uint8_t rx_buf[UDS_ISOTP_BUF_SIZE];
} UdsBusCtx;

static UdsBusCtx s_uds[CAN_IF_BUS_COUNT];
static uint32_t s_isotp_time_us;
static CanUser_UdsConfig s_uds_cfg;

int isotp_user_send_can(const uint32_t arbitration_id, const uint8_t *data, const uint8_t size
#ifdef ISO_TP_USER_SEND_CAN_ARG
                        ,
                        void *arg
#endif
)
{
#ifdef ISO_TP_USER_SEND_CAN_ARG
  CanIfBus bus = (CanIfBus)(uintptr_t)arg;
#else
  CanIfBus bus = CAN_IF_BUS1;
#endif

  if (data == NULL || size == 0U || size > 8U || bus >= CAN_IF_BUS_COUNT)
  {
    return ISOTP_RET_ERROR;
  }

  return CanIf_Send(bus, arbitration_id, false, data, size) ? ISOTP_RET_OK : ISOTP_RET_ERROR;
}

void isotp_user_debug(const char *message, ...)
{
  (void)message;
}

uint32_t isotp_user_get_us(void)
{
  return s_isotp_time_us;
}

static void uds_bus_poll(CanIfBus bus)
{
  UdsBusCtx *ctx = &s_uds[bus];
  CanIfFrame frame;
  uint8_t payload[UDS_ISOTP_BUF_SIZE];
  uint8_t response[UDS_ISOTP_BUF_SIZE];
  uint32_t out_size;
  uint32_t resp_len;

  isotp_poll(&ctx->link);

  while (CanIf_RecvUds(bus, &frame))
  {
    if (!frame.extended &&
        (frame.id == s_uds_cfg.rx_phy_id || frame.id == s_uds_cfg.rx_func_id))
    {
      isotp_on_can_message(&ctx->link, frame.data, frame.dlc);
    }
  }

  if (isotp_receive(&ctx->link, payload, sizeof(payload), &out_size) == ISOTP_RET_OK)
  {
    resp_len = 0U;
    if (CanUser_Uds_HandleRequest(bus, payload, out_size, response, &resp_len) == 0 &&
        resp_len > 0U)
    {
      (void)isotp_send(&ctx->link, response, resp_len);
    }
  }
}

static void uds_task(void *argument)
{
  (void)argument;

  vTaskDelay(pdMS_TO_TICKS(150));

  CanUser_Uds_GetConfig(&s_uds_cfg);

  for (CanIfBus bus = CAN_IF_BUS1; bus < CAN_IF_BUS_COUNT; bus = (CanIfBus)(bus + 1))
  {
    UdsBusCtx *ctx = &s_uds[bus];

    isotp_init_link(&ctx->link, s_uds_cfg.tx_id, ctx->tx_buf, sizeof(ctx->tx_buf),
                    ctx->rx_buf, sizeof(ctx->rx_buf));
#ifdef ISO_TP_USER_SEND_CAN_ARG
    ctx->link.user_send_can_arg = (void *)(uintptr_t)bus;
#endif
  }

  for (;;)
  {
    s_isotp_time_us += 1000U;

    for (CanIfBus bus = CAN_IF_BUS1; bus < CAN_IF_BUS_COUNT; bus = (CanIfBus)(bus + 1))
    {
      uds_bus_poll(bus);
    }

    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

void UdsApp_Init(void)
{
  (void)xTaskCreate(uds_task, "uds", 768U, NULL, tskIDLE_PRIORITY + 3, NULL);
}

void UdsApp_Poll(CanIfBus bus)
{
  if (bus >= CAN_IF_BUS_COUNT)
  {
    return;
  }

  s_isotp_time_us += 1000U;
  uds_bus_poll(bus);
}
