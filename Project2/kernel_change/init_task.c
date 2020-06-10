/* 
 * This file is modified to intialize my_mm_limits.
 * You can search the modification by "my change begin".
 * 
 * modified by Zhicun Chen
 * 2020/06/10
 * 
 */

/*
 *  linux/arch/arm/kernel/init_task.c
 */
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/init_task.h>
#include <linux/mqueue.h>
#include <linux/uaccess.h>
//my change begin
#include <linux/slab.h>
//my change end 

#include <asm/pgtable.h>

static struct signal_struct init_signals = INIT_SIGNALS(init_signals);
static struct sighand_struct init_sighand = INIT_SIGHAND(init_sighand);
/*
 * Initial thread structure.
 *
 * We need to make sure that this is 8192-byte aligned due to the
 * way process stacks are handled. This is done by making sure
 * the linker maps this in the .text segment right after head.S,
 * and making head.S ensure the proper alignment.
 *
 * The things we do for performance..
 */
union thread_union init_thread_union __init_task_data =
	{ INIT_THREAD_INFO(init_task) };

/*
 * Initial task structure.
 *
 * All other task structs will be allocated on slabs in fork.c
 */
struct task_struct init_task = INIT_TASK(init_task);

//my change begin
// just initialize my_mm_limits and export it
struct MMLimits my_mm_limits={
	.uid={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
	.mm_max={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}
};

EXPORT_SYMBOL(my_mm_limits);
//my change end

EXPORT_SYMBOL(init_task);
