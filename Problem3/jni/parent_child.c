#include <linux/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/wait.h>

int main()
{
    pid_t parent=getpid();
    pid_t child=fork();

    if(child==-1)
    {
        printf("opps, something wrong happened during forling\n");
        return -1;
    }

    if (child>0)     // in parent process
    {
        printf("518030910173 parent's pid is %d\n",parent);
        wait(NULL);
        printf("child complete!\n");
    }
    
    if(child==0)      // in child process
    {
        printf("518030910173 child's pid is %d\n",getpid());
        execl("./test_pstreeARM","./test_pstreeARM",NULL);
        exit(0);
    }
    
    return 0;
}