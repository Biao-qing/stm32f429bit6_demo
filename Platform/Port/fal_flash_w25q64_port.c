#include "w25q64.h"

#include <fal_cfg.h>
#include <fal.h>

static int w25q64_fal_init(void)
{
  return W25Q64_Init() ? 0 : -1;
}

static int w25q64_fal_read(long offset, uint8_t *buf, size_t size)
{
  if (!W25Q64_Read((uint32_t)offset, buf, (uint32_t)size))
  {
    return -1;
  }

  return (int)size;
}

static int w25q64_fal_write(long offset, const uint8_t *buf, size_t size)
{
  if (!W25Q64_Write((uint32_t)offset, buf, (uint32_t)size))
  {
    return -1;
  }

  return (int)size;
}

static int w25q64_fal_erase(long offset, size_t size)
{
  uint32_t addr = (uint32_t)offset;
  uint32_t end = addr + (uint32_t)size;

  if (addr >= W25Q64_CAPACITY)
  {
    return -1;
  }

  addr -= addr % W25Q64_SECTOR_SIZE;
  if (end > W25Q64_CAPACITY)
  {
    end = W25Q64_CAPACITY;
  }

  while (addr < end)
  {
    if (!W25Q64_EraseSector(addr))
    {
      return -1;
    }
    addr += W25Q64_SECTOR_SIZE;
  }

  return (int)size;
}

struct fal_flash_dev w25q64_flash = {
    .name = W25Q64_FAL_DEV_NAME,
    .addr = 0,
    .len = W25Q64_CAPACITY,
    .blk_size = W25Q64_SECTOR_SIZE,
    .ops = {w25q64_fal_init, w25q64_fal_read, w25q64_fal_write, w25q64_fal_erase},
    .write_gran = 1,
};
