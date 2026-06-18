#ifndef W25Q64_H
#define W25Q64_H

#include <stdbool.h>
#include <stdint.h>

#define W25Q64_PAGE_SIZE   256U
#define W25Q64_SECTOR_SIZE 4096U
#define W25Q64_CAPACITY    (8U * 1024U * 1024U)

bool W25Q64_Init(void);
bool W25Q64_ReadJedecId(uint8_t id[3]);
bool W25Q64_Read(uint32_t addr, uint8_t *buf, uint32_t len);
bool W25Q64_Write(uint32_t addr, const uint8_t *buf, uint32_t len);
bool W25Q64_EraseSector(uint32_t addr);

#endif /* W25Q64_H */
