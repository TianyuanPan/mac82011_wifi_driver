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

struct _index_t {
	int index;
	struct _index_t *next;
};

typedef struct _index_t index_t;

#define LOCK_MAC_TABLE() do { \
	mutex_lock(&mac_table_lock); \
} while (0)

#define UNLOCK_MAC_TABLE() do { \
	mutex_unlock(&mac_table_lock); \
} while (0)

extern struct mutex mac_table_lock;

#define TAB_INDEX_SIZE       1024
#define MAC_PROBE_INFO_SIZE  7

#define PROOC_ENTRY_NAME  "mac_probe_info"

extern index_t mac_table_index;
extern index_t *cur_index;
extern unsigned char procfs_mac_table_info[][MAC_PROBE_INFO_SIZE];

int  proc_mac_table_init(void);
void proc_mac_table_exit(void);

#endif /* PROCFS_MAC_TABLE_H_ */
