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

#define MY_MM_LENGTH 10
#define __NR_my_oom_killer 382
#define OOM_SCORE_ADJ_MIN (-1000)

static int (*oldcall)(void);
static int my_oom_killer(void){
    struct task_struct *p;
    int mm_alloc[MY_MM_LENGTH];
    struct task_struct* max_rss_process[MY_MM_LENGTH];
    int max_rss[MY_MM_LENGTH];
    int i,found,index;

    read_lock(&tasklist_lock);
my_oom_begin:
	for(i=0;i<MY_MM_LENGTH;i++){
		mm_alloc[i]=0;
		max_rss_process[i] = NULL;
		max_rss[i]=0;
	}
	for_each_process(p) {
		found=0;
		for(i=0;i<MY_MM_LENGTH;i++){
			if (p->cred->uid==my_mm_limits.uid[i]){
				if (max_rss_process[i] == NULL){
					if (p->mm){
						max_rss_process[i]=p;
						max_rss[i]=get_mm_rss(p->mm) * 4096;
					}
				}
				else{
					if (p->mm){
						if ((get_mm_rss(p->mm) * 4096)>max_rss[i]){
							max_rss_process[i]=p;
							max_rss[i]=get_mm_rss(p->mm) * 4096;// FIX
						}
					}
				}
				found=1;
				index=i;
				break;
			}
		}
		if (found){
            if (p->mm){
			    mm_alloc[index]+=get_mm_rss(p->mm) * 4096;
            }
		}
	}
	for(i=0;i<MY_MM_LENGTH;i++){
		if (my_mm_limits.uid[i]==-1){continue;}
		else{
			if (mm_alloc[i] <= my_mm_limits.mm_max[i]){continue;}
			else{
				task_lock(max_rss_process[i]);
					printk(KERN_ERR "uid=%d,\tuRSS=%d,\tmm_max=%d,\tpid=%d,\tpRSS=%d\n",
						max_rss_process[i]->cred->uid,mm_alloc[i],my_mm_limits.mm_max[i],max_rss_process[i]->pid,max_rss[i]);
				task_unlock(max_rss_process[i]);
				
				for_each_process(p) {
					if (p->mm == max_rss_process[i]->mm && !same_thread_group(p,max_rss_process[i]) 
						&& !(p->flags & PF_KTHREAD)) {
							if (p->signal->oom_score_adj == OOM_SCORE_ADJ_MIN)
								continue;
							
							task_lock(p);
							printk(KERN_ERR "Kill process %d (%s) sharing same memory\n",p->pid, p->comm);
							task_unlock(p);
							do_send_sig_info(SIGKILL, SEND_SIG_FORCED, p, true);
						}
				}
				
				set_tsk_thread_flag(max_rss_process[i], TIF_MEMDIE);
				do_send_sig_info(SIGKILL, SEND_SIG_FORCED, max_rss_process[i], true);
				mm_alloc[i] -= max_rss[i];
			}
		}
	}
	for (i=0;i<MY_MM_LENGTH;i++){
		if (my_mm_limits.uid[i]==-1){continue;}
		else{
			if (mm_alloc[i] > my_mm_limits.mm_max[i]){
				goto my_oom_begin;
			}
		}
	}
	read_unlock(&tasklist_lock);

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