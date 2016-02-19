/*
 * procfs_mac_table.c
 *
 *  Created on: Feb 18, 2016
 *      Author: TianyuanPan
 */

#include <linux/vmalloc.h>
#include "procfs_mac_table.h"


struct mutex mac_table_lock;

index_t mac_table_index;
index_t *cur_index = NULL;

unsigned char procfs_mac_table_info[TAB_INDEX_SIZE][MAC_PROBE_INFO_SIZE] = {{0}};

static struct proc_dir_entry *mac_table_entry;


static int init_index_t_size(index_t *index, int size)
{
	int i;
	index_t *head = NULL,
	*move = NULL,
	*new = NULL;
	head = index;
	move = index;
	new = NULL;

	i = size;
	if ( i < 1 ) {
		printk("Init Index size error, size should be great than 1");
		return -1;
	}
	if (!move) {
		printk("Init Index size error, index is NULL");
		return -1;
	}
	head->index = 0;
	head->next = index;

	for (i = 1; i < size; i++) {
		new = (index_t*)vmalloc(sizeof(index_t));
		if(!new) {
			printk("Init index_t size vmalloc error");
			return -1;
		}
		new->index = i;
		new->next = head;
		move->next = new;
		move = new;
	}

	return 0;
}

static void destory_index_t(index_t *index)
{
	index_t *move = NULL,
	*head = NULL,
	*p = NULL;

	head = index;
	move = head->next;

	while(move != head) {
		p = move->next;
		vfree(move);
		move = p;
	}
}



static int proc_mac_table_show(struct seq_file *seq, void *v)
{

	index_t *item = NULL,
	*head = NULL;
	head = &mac_table_index;
	item = head;

	LOCK_MAC_TABLE();

	do {
		if(procfs_mac_table_info[item->index][0] < 0) {

			seq_printf(seq,"[%d] %02x:%02x:%02x:%02x:%02x:%02x\n",
					procfs_mac_table_info[item->index][0],
					procfs_mac_table_info[item->index][1],
					procfs_mac_table_info[item->index][2],
					procfs_mac_table_info[item->index][3],
					procfs_mac_table_info[item->index][4],
					procfs_mac_table_info[item->index][5],
					procfs_mac_table_info[item->index][6]);

			memset(procfs_mac_table_info[item->index], 0, MAC_PROBE_INFO_SIZE);
		}

		item = item->next;

	}while(item != head);

	UNLOCK_MAC_TABLE();

	return 0;
}

static size_t proc_mac_table_write(struct file *file, const char __user *buffer,
		                         size_t count, loff_t *ppos)
{
	return 0;
}


static int proc_mac_table_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_mac_table_show, PDE_DATA(inode));
}

static const struct file_operations procfs_mac_table_info_fops =
{
		.owner = THIS_MODULE,
		.open = proc_mac_table_open,
		.read = seq_read,
		.write = proc_mac_table_write,
		.llseek = seq_lseek,
		.release = single_release,
};

int proc_mac_table_init(void)
{
	mutex_init(&mac_table_lock);

	if (init_index_t_size(&mac_table_index, TAB_INDEX_SIZE) != 0)
		return -1;

	cur_index = &mac_table_index;

	mac_table_entry = proc_create_data(PROOC_ENTRY_NAME, 0444, NULL, &procfs_mac_table_info_fops, procfs_mac_table_info);

	if(!mac_table_entry){
		printk(KERN_ERR "proc_mac_init_ERR: proc_create_data mac_probe_info error!!!!\n");
		destory_index_t(&mac_table_index);
		return -1;
	}

	return 0;
}

void proc_mac_table_exit(void)
{
	if(mac_table_entry){
	   remove_proc_entry(PROOC_ENTRY_NAME, NULL);
	   destory_index_t(&mac_table_index);
	}
}


