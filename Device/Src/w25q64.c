#include "w25q64.h"

#include "main.h"
#include "spi.h"

#define W25Q64_CMD_WRITE_ENABLE  0x06U
#define W25Q64_CMD_READ_STATUS1  0x05U
#define W25Q64_CMD_READ_DATA     0x03U
#define W25Q64_CMD_PAGE_PROGRAM  0x02U
#define W25Q64_CMD_SECTOR_ERASE  0x20U
#define W25Q64_CMD_READ_JEDEC_ID 0x9FU

#define W25Q64_STATUS_BUSY 0x01U

#define W25Q64_CS_LOW() \
  HAL_GPIO_WritePin(W25Q64_CS_GPIO_Port, W25Q64_CS_Pin, GPIO_PIN_RESET)
#define W25Q64_CS_HIGH() \
  HAL_GPIO_WritePin(W25Q64_CS_GPIO_Port, W25Q64_CS_Pin, GPIO_PIN_SET)

static bool w25q64_tx(const uint8_t *tx, uint32_t len)
{
  return HAL_SPI_Transmit(&hspi3, (uint8_t *)tx, (uint16_t)len, HAL_MAX_DELAY) == HAL_OK;
}

static bool w25q64_rx(uint8_t *rx, uint32_t len)
{
  return HAL_SPI_Receive(&hspi3, rx, (uint16_t)len, HAL_MAX_DELAY) == HAL_OK;
}

static bool w25q64_wait_busy(void)
{
  uint8_t cmd = W25Q64_CMD_READ_STATUS1;
  uint8_t status = 0xFFU;

  W25Q64_CS_LOW();
  if (!w25q64_tx(&cmd, 1U))
  {
    W25Q64_CS_HIGH();
    return false;
  }

  do
  {
    if (!w25q64_rx(&status, 1U))
    {
      W25Q64_CS_HIGH();
      return false;
    }
    HAL_Delay(1U);
  } while ((status & W25Q64_STATUS_BUSY) != 0U);

  W25Q64_CS_HIGH();
  return true;
}

static bool w25q64_write_enable(void)
{
  uint8_t cmd = W25Q64_CMD_WRITE_ENABLE;

  W25Q64_CS_LOW();
  if (!w25q64_tx(&cmd, 1U))
  {
    W25Q64_CS_HIGH();
    return false;
  }
  W25Q64_CS_HIGH();
  return true;
}

bool W25Q64_Init(void)
{
  uint8_t id[3];

  W25Q64_CS_HIGH();
  return W25Q64_ReadJedecId(id);
}

bool W25Q64_ReadJedecId(uint8_t id[3])
{
  uint8_t cmd = W25Q64_CMD_READ_JEDEC_ID;

  W25Q64_CS_LOW();
  if (!w25q64_tx(&cmd, 1U) || !w25q64_rx(id, 3U))
  {
    W25Q64_CS_HIGH();
    return false;
  }
  W25Q64_CS_HIGH();
  return true;
}

bool W25Q64_Read(uint32_t addr, uint8_t *buf, uint32_t len)
{
  uint8_t header[4];

  if ((buf == NULL) || (len == 0U) || ((addr + len) > W25Q64_CAPACITY))
  {
    return false;
  }

  header[0] = W25Q64_CMD_READ_DATA;
  header[1] = (uint8_t)(addr >> 16);
  header[2] = (uint8_t)(addr >> 8);
  header[3] = (uint8_t)(addr);

  W25Q64_CS_LOW();
  if (!w25q64_tx(header, sizeof(header)) || !w25q64_rx(buf, len))
  {
    W25Q64_CS_HIGH();
    return false;
  }
  W25Q64_CS_HIGH();
  return true;
}

bool W25Q64_Write(uint32_t addr, const uint8_t *buf, uint32_t len)
{
  uint32_t offset = 0U;

  if ((buf == NULL) || (len == 0U) || ((addr + len) > W25Q64_CAPACITY))
  {
    return false;
  }

  while (offset < len)
  {
    uint32_t page_offset = addr % W25Q64_PAGE_SIZE;
    uint32_t chunk = W25Q64_PAGE_SIZE - page_offset;

    if (chunk > (len - offset))
    {
      chunk = len - offset;
    }

    if (!w25q64_write_enable())
    {
      return false;
    }

    uint32_t current_addr = addr + offset;
    uint8_t header[4];
    header[0] = W25Q64_CMD_PAGE_PROGRAM;
    header[1] = (uint8_t)(current_addr >> 16);
    header[2] = (uint8_t)(current_addr >> 8);
    header[3] = (uint8_t)(current_addr);

    W25Q64_CS_LOW();
    if (!w25q64_tx(header, sizeof(header)) ||
        !w25q64_tx(&buf[offset], chunk))
    {
      W25Q64_CS_HIGH();
      return false;
    }
    W25Q64_CS_HIGH();

    if (!w25q64_wait_busy())
    {
      return false;
    }

    offset += chunk;
  }

  return true;
}

bool W25Q64_EraseSector(uint32_t addr)
{
  uint8_t header[4];

  if (addr >= W25Q64_CAPACITY)
  {
    return false;
  }

  if (!w25q64_write_enable())
  {
    return false;
  }

  header[0] = W25Q64_CMD_SECTOR_ERASE;
  header[1] = (uint8_t)(addr >> 16);
  header[2] = (uint8_t)(addr >> 8);
  header[3] = (uint8_t)(addr);

  W25Q64_CS_LOW();
  if (!w25q64_tx(header, sizeof(header)))
  {
    W25Q64_CS_HIGH();
    return false;
  }
  W25Q64_CS_HIGH();

  return w25q64_wait_busy();
}
