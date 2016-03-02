/*
 * procfs_mac_table.c
 *
 *  Created on: Feb 19, 2016
 *      Author: TianyuanPan
 */


#include <linux/vmalloc.h>
#include <linux/spinlock.h>
#include <linux/seq_file.h>
#include <linux/time.h>
#include "procfs_mac_table.h"

spinlock_t mac_table_lock;

index_t mac_table_index;
index_t *cur_index = NULL;

mac_signal_t procfs_mac_table_info[TABLE_MAX_LEN];

static struct proc_dir_entry *mac_table_entry;


void set_timespec_to_table_elem(mac_signal_t *elem)
{
	struct timespec tsc = current_kernel_time();
	short milliseconds = (short)(tsc.tv_nsec / 1000L / 1000L);
	short microseconds = (short)(tsc.tv_nsec / 1000L % 1000L);
	short nanoseconds  = (short)(tsc.tv_nsec % 1000L % 1000L);
	if (!elem)
		return;
	elem->seconds = tsc.tv_sec;
	elem->milliseconds = milliseconds;
	elem->microseconds = microseconds;
	elem->nanoseconds  = nanoseconds;
}

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


/*
 *  s:     almost always ignored
 *  pos:   integer position indicateing where to start
 *         need not be a byte position
 */
static void *seq_seq_start(struct seq_file *s, loff_t *pos)
{
//    PDEBUG("position is %d/n", *pos);
    if (*pos >= TABLE_MAX_LEN)
        return NULL;
    return  procfs_mac_table_info + *pos;
}

/*
 *  v:       is the iterator as returned from previous call to start or next
 *  return:  NULL means nothing left
 */
static void *seq_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
//    PDEBUG("next: %d/n", *pos);
    (*pos) = ++(*pos);
    if (*pos >= TABLE_MAX_LEN)
        return NULL;
    return  procfs_mac_table_info + *pos;
}

static void seq_seq_stop(struct seq_file *s, void *v)
{
    /* Actually, there's nothing to do here */
	return;
}

static int seq_seq_show(struct seq_file *seq, void *v)
{
	mac_signal_t *ptr = (mac_signal_t *)v;

	LOCK_MAC_TABLE();

	if( ptr->c_signal < 0) {


			seq_printf(seq, "[%d] %02x:%02x:%02x:%02x:%02x:%02x %ld%03d%03d.%d\n",
					        ptr->c_signal,
					        ptr->c_mac[0],
					        ptr->c_mac[1],
					        ptr->c_mac[2],
					        ptr->c_mac[3],
					        ptr->c_mac[4],
					        ptr->c_mac[5],
					        ptr->seconds,
					        ptr->milliseconds,
					        ptr->microseconds,
					        ptr->nanoseconds
					    );

			ptr->c_signal = 0;
			memset(ptr->c_mac, 0, MAC_ADDR_LEN);
	}

	UNLOCK_MAC_TABLE();
    return 0;
}
/*
 * Tie the sequence operators up.
 */
static struct seq_operations seq_seq_ops = {
    .start = seq_seq_start,
    .next  = seq_seq_next,
    .stop  = seq_seq_stop,
    .show  = seq_seq_show
};

/*
 * Now to implement the /proc file we need only make an open
 * method which sets up the sequence operators.
 */
static int seq_proc_open(struct inode *inode, struct file *file)
{
    return seq_open(file, &seq_seq_ops);
}

static size_t proc_mac_table_write(struct file *file, const char __user *buffer,
		                         size_t count, loff_t *ppos)
{
	return 0;
}

static const struct file_operations procfs_mac_table_info_fops =
{
		.owner = THIS_MODULE,
		.open = seq_proc_open,
		.read = seq_read,
		.write = proc_mac_table_write,
		.llseek = seq_lseek,
		.release = seq_release,
};

int proc_mac_table_init(void)
{
	spin_lock_init(&mac_table_lock);

	if (init_index_t_size(&mac_table_index, TABLE_MAX_LEN) != 0)
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


