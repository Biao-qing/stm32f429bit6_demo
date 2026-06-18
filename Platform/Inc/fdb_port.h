#ifndef FDB_PORT_H
#define FDB_PORT_H

#include <flashdb.h>

void fdb_port_print(const char *fmt, ...);
void fdb_port_lock(fdb_db_t db);
void fdb_port_unlock(fdb_db_t db);
void fdb_port_mutex_init(void);
fdb_time_t fdb_port_get_time(void);

#endif /* FDB_PORT_H */
