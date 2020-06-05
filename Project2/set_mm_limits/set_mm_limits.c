#include <linux/module.h>
#include <linux/mm.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/unistd.h>
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Zhicun Chen");

#define MY_MM_LENGTH 10
#define __NR_set_mm_limitscall 356

static int (*oldcall)(void);
static int set_mm_limits(int uid,int mm_max){
    int i;
    int full=0;
    /*
    printk(KERN_INFO "---------------------");
    for(i=0;i<MY_MM_LENGTH;i++){
            printk(KERN_INFO "uid=%d,mm_max=%d",my_mm_limits.uid[i],my_mm_limits.mm_max[i]);
    }
    printk(KERN_INFO "---------------------");
    */
    if(uid==0){full=-1;}
    else{
    for(i=0;i<MY_MM_LENGTH;i++){
        if(my_mm_limits.uid[i]!=-1){
            if(my_mm_limits.uid[i]!=uid){
                continue;
            }
            else{
                int j;
                for(j=i;(j<(MY_MM_LENGTH-1))&&(my_mm_limits.uid[j+1]!=-1);j++){
                    my_mm_limits.uid[j]=my_mm_limits.uid[j+1];
                    my_mm_limits.mm_max[j]=my_mm_limits.mm_max[j+1];
                }
                my_mm_limits.mm_max[j]=mm_max;
                my_mm_limits.uid[j]=uid;
                //my_mm_limits.mm_max[i]=mm_max;
                //update last use
                break;
            }
        }
        else{
            my_mm_limits.uid[i]=uid;
            my_mm_limits.mm_max[i]=mm_max;
            break;
        }        
    }
    if(i==MY_MM_LENGTH){
        full=1;
        printk(KERN_ERR "The MMLimits is full! We will delete the earliest limits!\n");
    }

    if(full){
        for(i=0;i<MY_MM_LENGTH-1;i++){
            my_mm_limits.uid[i]=my_mm_limits.uid[i+1];
            my_mm_limits.mm_max[i]=my_mm_limits.mm_max[i+1];
        }
        my_mm_limits.uid[9]=uid;
        my_mm_limits.mm_max[9]=mm_max;
    }

    // print all mm limits
    for(i=0;i<MY_MM_LENGTH;i++){
        if(my_mm_limits.uid[i]!=-1){
            printk(KERN_INFO "uid=%d,mm_max=%d",my_mm_limits.uid[i],my_mm_limits.mm_max[i]);
        }
    }}

    printk(KERN_INFO "\n");

    return full;
}

static int addsyscall_init(void){
    long *syscall=(long*)0xc000d8c4;
    oldcall=(int(*)(void))(syscall[__NR_set_mm_limitscall]);
    syscall[__NR_set_mm_limitscall]=(unsigned long)set_mm_limits;
    printk(KERN_INFO "module load!\n");
    return 0;
}

static void addsyscall_exit(void){
    long *syscall=(long*)0xc000d8c4;
    syscall[__NR_set_mm_limitscall]=(unsigned long)oldcall;
    printk(KERN_INFO "module exit!\n");
}

module_init(addsyscall_init);
module_exit(addsyscall_exit);