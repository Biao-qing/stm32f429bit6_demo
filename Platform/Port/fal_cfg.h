#ifndef FAL_CFG_H
#define FAL_CFG_H

#define FAL_DEBUG 0
#define FAL_PART_HAS_TABLE_CFG

#define W25Q64_FAL_DEV_NAME "w25q64"

extern struct fal_flash_dev w25q64_flash;

#define FAL_FLASH_DEV_TABLE \
  {                         \
      &w25q64_flash,          \
  }

#ifdef FAL_PART_HAS_TABLE_CFG
#define FAL_PART_TABLE                                                                  \
  {                                                                                     \
      {FAL_PART_MAGIC_WORD, "fdb_tsdb", W25Q64_FAL_DEV_NAME, 0, 2 * 1024 * 1024, 0},    \
      {FAL_PART_MAGIC_WORD, "fdb_kvdb", W25Q64_FAL_DEV_NAME, 2 * 1024 * 1024, 512 * 1024, 0}, \
  }
#endif

#endif /* FAL_CFG_H */
