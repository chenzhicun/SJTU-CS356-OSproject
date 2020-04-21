#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/unistd.h>
#include <linux/list.h>    // for list defined in kernel
#include <linux/slab.h>    // for kfree and kmalloc
#include <linux/uaccess.h> // for copy_to_user
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Zhicun Chen");
#define __NR_ptreecall 356
#define BUFFER 2048

struct prinfo
{
    pid_t parent_pid;       // process id of parent
    pid_t pid;              // process id
    pid_t first_child_pid;  // pid of youngest child
    pid_t next_sibling_pid; // pid of older sibling
    long state;             // current state of process
    long uid;               // user id of process owner
    /*in sched.h this length is 16, here I change it to 16 to fit the defination in sched.h*/
    /*However, now it works fine....... I'm a little confused.*/
    char comm[64];          // name of program executed
};

void tree_dfs(struct task_struct *task, struct prinfo *buf, int *nr)
{
    struct list_head *head;
    struct task_struct *task_p;

    /*do some copy between task_struct and buffer we defined*/
    buf[*nr].parent_pid = (task->parent) ? task->parent->pid : 0;
    buf[*nr].pid = task->pid;
    buf[*nr].first_child_pid = (list_empty(&(task->children))) ? 0 : list_entry(task->children.next, struct task_struct, sibling)->pid;
    buf[*nr].next_sibling_pid = (list_empty(&(task->sibling))) ? 0 : list_entry(task->sibling.next, struct task_struct, sibling)->pid;
    buf[*nr].state=task->state;
    buf[*nr].uid=task->cred->uid;
    strcpy(buf[*nr].comm,task->comm);
    //get_task_comm(buf[*nr].comm,task);
    /*finish copy*/

    /*recursively call tree_dfs to go through*/
    (*nr)+=1;
    list_for_each(head,&(task->children))
    {
        task_p=list_entry(head,struct task_struct,sibling);
        tree_dfs(task_p,buf,nr);
    }
}

static int (*oldcall)(void);
static int ptree(struct prinfo *buf, int *nr)
{
    struct prinfo *_buf=(struct prinfo*)kmalloc(sizeof(struct prinfo)*BUFFER,GFP_KERNEL);
    int _nr=0;
    /*prevent data strcture from changing*/
    read_lock(&tasklist_lock);
    tree_dfs(&init_task,_buf,&_nr);
    read_unlock(&tasklist_lock);
    
    if(copy_to_user(nr,&_nr,sizeof(int)))
    {
        printk(KERN_ERR "Something wrong happened during copy nr!\n");
        return 1;
    }
    if(copy_to_user(buf,_buf,sizeof(struct prinfo)*BUFFER))
    {
        printk(KERN_ERR "Something wrong happened during copy buf!\n");
        return 1;
    }

    kfree(_buf);
    return 0;
}

static int addsyscall_init(void)
{
    long *syscall = (long *)0xc000d8c4;
    oldcall = (int (*)(void))(syscall[__NR_ptreecall]);
    syscall[__NR_ptreecall] = (unsigned long)ptree;
    printk(KERN_INFO "my pstree module load!\n");
    return 0;
}

static void addsyscall_exit(void)
{
    long *syscall = (long *)0xc000d8c4;
    syscall[__NR_ptreecall] = (unsigned long)oldcall;
    printk(KERN_INFO "my pstree module exit!\n");
}
module_init(addsyscall_init);
module_exit(addsyscall_exit);