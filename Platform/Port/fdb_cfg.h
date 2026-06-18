#ifndef FDB_CFG_H
#define FDB_CFG_H

#define FDB_USING_KVDB
#define FDB_USING_TSDB
#define FDB_USING_FAL_MODE

/* External NOR flash (W25Q64) write granularity is 1 bit */
#define FDB_WRITE_GRAN 1

/* #define FDB_DEBUG_ENABLE */

void fdb_port_print(const char *fmt, ...);
#define FDB_PRINT(...) fdb_port_print(__VA_ARGS__)

#endif /* FDB_CFG_H */
