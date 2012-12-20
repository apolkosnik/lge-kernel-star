/*
 * arch/arm/mach-tegra/board-star-nv.c
 * Keys configuration for Nvidia tegra2 x2 platform.
 *
 * Copyright (C) 2011 NVIDIA, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */

#include <linux/signal.h>
#include <linux/personality.h>
#include <linux/kallsyms.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/hardirq.h>
#include <linux/kdebug.h>
#include <linux/module.h>
#include <linux/kexec.h>
#include <linux/delay.h>
#include <linux/init.h>

#include <asm/atomic.h>
#include <asm/cacheflush.h>
#include <asm/system.h>
#include <asm/unistd.h>
#include <asm/traps.h>
#include <asm/unwind.h>

#include <linux/fs.h>
#include <linux/syscalls.h>
#include <lge/board-star-nv.h>
#include <linux/genhd.h>

static char nvdata_partition[] = LGE_NVDATA_PARTITION;

static void __init lge_nvdata_partition_init(void)
{
	dev_t dsk = blk_lookup_devt("mmcblk0", 0);
	if (!dsk) {
		pr_err("%s: %d FAIL to detect mmcblk0\n", __func__, dsk);
	} else {
		int partno;
		struct gendisk *disk = get_gendisk(dsk, &partno);
		int partitions[] = {
			5, /* for ICS partition layout */
			3, /* for GB partition layout */
			0,
		};

		if (disk) {
			int i;
			struct hd_struct *part;
			struct disk_part_tbl *ptbl;

			ptbl = disk->part_tbl;
			for (i = 0; partitions[i]; i++) {
				part = ptbl->part[partitions[i]];
				pr_info("%s: mmcblk0p%d %lu\n", __func__, partitions[i], (long)part->nr_sects);

				if (part->nr_sects == 4096) {
					/* the sector size of the misc partition is 4096 */
					sprintf(nvdata_partition, "/dev/block/mmcblk0p%d", partitions[i]);
					pr_info("%s: misc partition found %s\n", __func__, nvdata_partition);
					break;
				}
			}
			put_disk(disk);
		} else {
			pr_err("%s: %d FAIL to get_gendisk mmcblk0\n", __func__);
		}
	}
}
late_initcall(lge_nvdata_partition_init);

int lge_nvdata_raw_read(int offset, char* buf, int size)
{
	if(size == 0) return 0;

	int h_file = 0;
	int ret = 0;
	mm_segment_t oldfs = get_fs();
	set_fs(KERNEL_DS);
	h_file = sys_open(nvdata_partition, O_RDWR,0);

	if(h_file >= 0)
	{
		printk("read NV size = %d, offset = %d\n",size, offset);	
		sys_lseek( h_file, offset, 0 );

		ret = sys_read( h_file, buf, size);
		
		printk("read NV ret = %d\n",ret);	
		
		if( ret != size )
		{
			printk("Can't read  NVDATA.\n");
			return ret;
		}

		sys_close(h_file);
	}
	else
	{
		printk("Can't open  nvdata partition handle = %d.\n",h_file);
		return 0;
	}
	set_fs(oldfs);
	if (size > 1)
	       printk("read NV offset = %d, message = %s(%x)[%d]\n", offset, buf, *buf, *buf);
	else	
       	printk("read NV offset = %d, message = %x[%d]\n", offset, *buf, *buf);
	   
	return size;
}

int lge_nvdata_raw_write(int offset, char* buf, int size)

{
	if(size == 0) return 0;

	int h_file = 0;
	int ret = 0;

	mm_segment_t oldfs = get_fs();
	set_fs(KERNEL_DS);
	h_file = sys_open(nvdata_partition, O_RDWR,0);

	if(h_file >= 0)
	{
		printk("write NV size = %d, offset = %d\n",size, offset);	
		sys_lseek( h_file, offset, 0 );

		ret = sys_write( h_file, buf, size);
		
		printk("write NV ret = %d\n",ret);	
		
		if( ret != size )
		{
			printk("Can't write  NVDATA.\n");
			return ret;
		}

		sys_close(h_file);
	}
	else
	{
		printk("Can't open  NVDATA partition handle = %d.\n",h_file);
		return 0;
	}
	set_fs(oldfs);

	sys_sync();
	
	if (size > 1)
       	printk("write NV offset = %d, message = %s(%x)[%d]\n", offset, buf, *buf, *buf);
	else
	       printk("write NV offset = %d, message = %x[%d]\n", offset, *buf, *buf);
	   
	return size;
}

int lge_nvdata_read(lge_nvdata_offset offset, char* buf, int size)
{
	printk("start lge_nvdata_read .\n");
	int pos = (int)offset ;
	return lge_nvdata_raw_read(pos,buf,size);
}

int lge_nvdata_write(lge_nvdata_offset offset, char* buf, int size)
{
	printk("start lge_nvdata_write .\n");
	int pos = (int)offset;
	return lge_nvdata_raw_write(pos,buf,size);
}


void lge_clean_nvdata_partition()
{
}
