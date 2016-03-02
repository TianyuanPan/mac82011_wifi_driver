/*
 * procfs_mac_table.h
 *
 *  Created on: Feb 19, 2016
 *      Author: TianyuanPan
 */

#ifndef PROCFS_MAC_TABLE_H_
#define PROCFS_MAC_TABLE_H_

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/mutex.h>

#define  TABLE_MAX_LEN    1024
#define  MAC_ADDR_LEN     6

#define PROOC_ENTRY_NAME  "mac_probe_info"

struct _mac_signal_t{
	char c_signal;
	long seconds;
	short milliseconds;
	short microseconds;
	short nanoseconds;
	unsigned char c_mac[MAC_ADDR_LEN];
};

typedef struct _mac_signal_t  mac_signal_t;

struct _index_t {
	int index;
	struct _index_t *next;
};

typedef struct _index_t index_t;

extern spinlock_t mac_table_lock;


#define LOCK_MAC_TABLE() do { \
	spin_lock(&mac_table_lock); \
} while (0)

#define UNLOCK_MAC_TABLE() do { \
	spin_unlock(&mac_table_lock); \
} while (0)


#define TRY_LOCK_MAC_TABLE() do { \
	if (!spin_trylock(&mac_table_lock)) return; \
} while (0)



extern index_t mac_table_index;
extern index_t *cur_index;
extern mac_signal_t procfs_mac_table_info[TABLE_MAX_LEN];

void set_timespec_to_table_elem(mac_signal_t *elem);

int  proc_mac_table_init(void);
void proc_mac_table_exit(void);

#endif /* PROCFS_MAC_TABLE_H_ */
