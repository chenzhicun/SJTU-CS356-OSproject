/*
 * This file is aimed at adding my oom-killer
 * as a syscall through a kernel module. The 
 * idea is actually the same as the kernel version.
 * But this can be called in a daemon program.
 * 
 * written by Zhicun Chen
 * 2020/06/10
 */ 

#include <linux/module.h>
#include <linux/mm.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/unistd.h>
#include <linux/signal.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Zhicun Chen");

#define __NR_my_oom_killer 382

// 0 means highest RSS, 1 means longest run time
#define OOM_KILLER_TYPE 1

static int (*oldcall)(void);
static int my_oom_killer(void){
	/*
	 * Just call corresponding oom_killer by OOM_KILLER_TYPE.
	 * As you can see, I use a switch statement here, so if 
	 * we design another efficient oom_killer, it's convinient
	 * to add it to this syscall.
	 */
	switch (OOM_KILLER_TYPE){
		case 0:
			oom_killer_highest_rss();
			break;
		case 1:
			oom_killer_longest_run_time();
			break;
		default:
			oom_killer_highest_rss();
			break;
	}

    return 0;
}

static int addsyscall_init(void){
    long *syscall=(long*)0xc000d8c4;
    oldcall=(int(*)(void))(syscall[__NR_my_oom_killer]);
    syscall[__NR_my_oom_killer]=(unsigned long)my_oom_killer;
    printk(KERN_INFO "module load!\n");
    return 0;
}

static void addsyscall_exit(void){
    long *syscall=(long*)0xc000d8c4;
    syscall[__NR_my_oom_killer]=(unsigned long)oldcall;
    printk(KERN_INFO "module exit!\n");
}

module_init(addsyscall_init);
module_exit(addsyscall_exit);