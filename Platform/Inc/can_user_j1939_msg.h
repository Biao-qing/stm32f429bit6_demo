#ifndef CAN_USER_J1939_MSG_H
#define CAN_USER_J1939_MSG_H

/*
 * J1939 报文矩阵（Message / Signal）— 用户在此维护 J1939 信号定义
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MSG_ADDR_CLAIMED_PGN   0x00EE00U
#define MSG_EEC1_PGN           0x00F004U
#define MSG_CCVS_PGN           0x00FEF1U
#define MSG_DM1_PGN            0x00FECAU

#define SIG_EEC1_ENG_SPEED_START_BIT  24U
#define SIG_EEC1_ENG_SPEED_LEN        16U
#define SIG_EEC1_ENG_SPEED_FACTOR     0.125f
#define SIG_EEC1_ENG_SPEED_OFFSET     0.0f

#define SIG_CCVS_VEH_SPEED_START_BIT  8U
#define SIG_CCVS_VEH_SPEED_LEN        16U
#define SIG_CCVS_VEH_SPEED_FACTOR     (1.0f / 256.0f)
#define SIG_CCVS_VEH_SPEED_OFFSET     0.0f

static inline uint64_t CanMsg_GetRawIntel(const uint8_t data[8], uint8_t start_bit, uint8_t length)
{
  uint64_t raw = 0U;
  uint8_t bit = start_bit;

  for (uint8_t i = 0U; i < length; i++)
  {
    uint8_t byte_idx = (uint8_t)(bit / 8U);
    uint8_t bit_idx = (uint8_t)(bit % 8U);
    if (byte_idx < 8U)
    {
      if ((data[byte_idx] & (uint8_t)(1U << bit_idx)) != 0U)
      {
        raw |= ((uint64_t)1U << i);
      }
    }
    bit++;
  }
  return raw;
}

static inline float CanMsg_RawToPhys(uint64_t raw, float factor, float offset)
{
  return ((float)raw * factor) + offset;
}

static inline float CanSig_EEC1_EngSpeed_rpm(const uint8_t data[8])
{
  uint64_t raw = CanMsg_GetRawIntel(data, SIG_EEC1_ENG_SPEED_START_BIT, SIG_EEC1_ENG_SPEED_LEN);
  return CanMsg_RawToPhys(raw, SIG_EEC1_ENG_SPEED_FACTOR, SIG_EEC1_ENG_SPEED_OFFSET);
}

static inline float CanSig_CCVS_VehSpeed_kmh(const uint8_t data[8])
{
  uint64_t raw = CanMsg_GetRawIntel(data, SIG_CCVS_VEH_SPEED_START_BIT, SIG_CCVS_VEH_SPEED_LEN);
  return CanMsg_RawToPhys(raw, SIG_CCVS_VEH_SPEED_FACTOR, SIG_CCVS_VEH_SPEED_OFFSET);
}

#ifdef __cplusplus
}
#endif

#endif /* CAN_USER_J1939_MSG_H */
