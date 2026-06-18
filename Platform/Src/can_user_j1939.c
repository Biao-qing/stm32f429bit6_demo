#include "can_user_j1939.h"
#include "can_user_j1939_msg.h"

#include "can_if.h"

void CanUser_J1939_FillEcu(CanIfBus bus, J1939 *j1939)
{
  if (j1939 == NULL)
  {
    return;
  }

  j1939->information_this_ECU.this_name.identity_number = 0x00001234U + (uint32_t)bus;
  j1939->information_this_ECU.this_name.manufacturer_code = 0x0123U;
  j1939->information_this_ECU.this_name.function_instance = 0U;
  j1939->information_this_ECU.this_name.ECU_instance = (uint8_t)bus;
  j1939->information_this_ECU.this_name.function = 130U;
  j1939->information_this_ECU.this_name.vehicle_system = 0U;
  j1939->information_this_ECU.this_name.arbitrary_address_capable = 1U;
  j1939->information_this_ECU.this_name.industry_group = 0U;
  j1939->information_this_ECU.this_name.vehicle_system_instance = 0U;
  j1939->information_this_ECU.this_ECU_address = (bus == CAN_IF_BUS1) ? 0x28U : 0x29U;
}

uint32_t CanUser_J1939_IdToPgn(uint32_t id)
{
  uint8_t pf = (uint8_t)((id >> 16) & 0xFFU);

  if (pf < 240U)
  {
    return ((id >> 8) & 0x3FF00U);
  }
  return ((id >> 8) & 0x3FFFFU);
}

bool CanUser_J1939_SendRaw(CanIfBus bus, uint32_t id, const uint8_t data[8])
{
  return CanIf_Send(bus, id, true, data, 8U);
}

void CanUser_J1939_OnPgnRx(CanIfBus bus, uint32_t pgn, uint8_t sa, const uint8_t data[8])
{
  (void)bus;

  switch (pgn)
  {
  case MSG_EEC1_PGN:
  {
    float eng_speed_rpm = CanSig_EEC1_EngSpeed_rpm(data);
    (void)eng_speed_rpm;
    (void)sa;
    break;
  }

  case MSG_CCVS_PGN:
  {
    float veh_speed_kmh = CanSig_CCVS_VehSpeed_kmh(data);
    (void)veh_speed_kmh;
    (void)sa;
    break;
  }

  default:
    (void)sa;
    (void)data;
    break;
  }
}

void CanUser_J1939_Periodic(CanIfBus bus, uint32_t tick_ms, J1939 *j1939)
{
  (void)bus;
  (void)j1939;
  (void)tick_ms;
}
