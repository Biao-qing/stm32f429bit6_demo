#include "j1939_app.h"

#include "can_if.h"
#include "can_user_j1939.h"

#include "Open_SAE_J1939/Open_SAE_J1939.h"
#include "Hardware/Hardware.h"
#include "SAE_J1939/SAE_J1939-81_Network_Management_Layer/Network_Management_Layer.h"

#include "FreeRTOS.h"
#include "task.h"

#include <string.h>

static J1939 s_j1939[CAN_IF_BUS_COUNT];
static bool s_j1939_ready[CAN_IF_BUS_COUNT];
static CanIfBus s_active_j1939_bus;

static void j1939_send_cb(uint32_t id, uint8_t dlc, uint8_t data[])
{
  (void)CanIf_Send(s_active_j1939_bus, id, true, data, dlc);
}

static void j1939_read_cb(uint32_t *id, uint8_t data[], bool *is_new_message)
{
  CanIfFrame frame;

  *is_new_message = false;
  if (!CanIf_RecvJ1939(s_active_j1939_bus, &frame))
  {
    return;
  }

  *id = frame.id;
  memcpy(data, frame.data, 8U);
  *is_new_message = true;
}

static void j1939_delay_cb(uint8_t milliseconds)
{
  vTaskDelay(pdMS_TO_TICKS(milliseconds));
}

static void j1939_init_ecu_info(J1939 *j1939, CanIfBus bus)
{
  memset(j1939, 0, sizeof(*j1939));

  CanUser_J1939_FillEcu(bus, j1939);

  j1939->information_this_ECU.this_identifications.ecu_identification.length_of_each_field = MAX_IDENTIFICATION;
  j1939->information_this_ECU.this_identifications.component_identification.length_of_each_field = MAX_IDENTIFICATION;
  j1939->from_other_ecu_identifications.ecu_identification.length_of_each_field = MAX_IDENTIFICATION;
  j1939->from_other_ecu_identifications.component_identification.length_of_each_field = MAX_IDENTIFICATION;

  j1939->this_proprietary.proprietary_A.total_bytes = MAX_PROPRIETARY_A;
  j1939->from_other_ecu_proprietary.proprietary_A.total_bytes = MAX_PROPRIETARY_A;
  for (int i = 0; i < (int)MAX_PROPRIETARY_B_PGNS; ++i)
  {
    j1939->this_proprietary.proprietary_B[i].total_bytes = MAX_PROPRIETARY_B;
    j1939->from_other_ecu_proprietary.proprietary_B[i].total_bytes = MAX_PROPRIETARY_B;
  }

  memset(j1939->other_ECU_address, 0xFF, sizeof(j1939->other_ECU_address));
  j1939->number_of_cannot_claim_address = 0U;
  j1939->number_of_other_ECU = 0U;
}

static void j1939_bus_startup(CanIfBus bus)
{
  s_active_j1939_bus = bus;
  j1939_init_ecu_info(&s_j1939[bus], bus);
  (void)SAE_J1939_Response_Request_Address_Claimed(&s_j1939[bus]);
  (void)SAE_J1939_Send_Request_Address_Claimed(&s_j1939[bus], 0xFFU);
  s_j1939_ready[bus] = true;
}

static void j1939_task(void *argument)
{
  uint32_t tick_ms = 0U;

  (void)argument;

  vTaskDelay(pdMS_TO_TICKS(100));

  CAN_Set_Callback_Functions(j1939_send_cb, j1939_read_cb, NULL, j1939_delay_cb);

  for (CanIfBus bus = CAN_IF_BUS1; bus < CAN_IF_BUS_COUNT; bus = (CanIfBus)(bus + 1))
  {
    j1939_bus_startup(bus);
  }

  for (;;)
  {
    for (CanIfBus bus = CAN_IF_BUS1; bus < CAN_IF_BUS_COUNT; bus = (CanIfBus)(bus + 1))
    {
      s_active_j1939_bus = bus;
      while (J1939App_ProcessOneFrame(bus))
      {
      }
      CanUser_J1939_Periodic(bus, tick_ms, &s_j1939[bus]);
    }

    tick_ms += 5U;
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

void J1939App_Init(void)
{
  (void)xTaskCreate(j1939_task, "j1939", 768U, NULL, tskIDLE_PRIORITY + 3, NULL);
}

bool J1939App_ProcessOneFrame(CanIfBus bus)
{
  ENUM_J1939_RX_MSG rx_msg;
  J1939 *j1939;

  if (bus >= CAN_IF_BUS_COUNT || !s_j1939_ready[bus])
  {
    return false;
  }

  j1939 = &s_j1939[bus];
  s_active_j1939_bus = bus;
  rx_msg = Open_SAE_J1939_Listen_For_Messages(j1939);

  if (rx_msg == RX_MSG_UNKNOWN || rx_msg == RX_MSG_NOT_SUPPORTED)
  {
    CanUser_J1939_OnPgnRx(bus, CanUser_J1939_IdToPgn(j1939->ID), (uint8_t)(j1939->ID & 0xFFU),
                          j1939->data);
  }

  return rx_msg != RX_MSG_NONE;
}
