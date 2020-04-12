#include <string.h>
#include <stdlib.h>
#include <linux/types.h>
#include <stdio.h>
#define BUFFER 2048

struct prinfo
{
    pid_t parent_pid;       // process id of parent
    pid_t pid;              // process id
    pid_t first_child_pid;  // pid of youngest child
    pid_t next_sibling_pid; // pid of older sibling
    long state;             // current state of process
    long uid;               // user id of process owner
    char comm[16];          // name of program executed
};

void print_pstree(struct prinfo *buffer, int nr)
{
    int *depth;
    depth = (int*)malloc(nr * sizeof(int));
    memset(depth, 0, nr);
    int i,j,k; //it seems C compiler doesn't support initialization in for loop

    printf("%s,%d,%ld,%d,%d,%d,%ld\n", buffer[0].comm, buffer[0].pid, buffer[0].state,
           buffer[0].parent_pid, buffer[0].first_child_pid, buffer[0].next_sibling_pid, buffer[0].uid);

    for (i = 1;i<nr;i++)
    {
        for(j=i-1;j>=0;j--)
        {
            if(buffer[i].parent_pid==buffer[j].pid)
            {
                depth[i]=depth[j]+1;
                break;
            }
        }

        for(k=depth[i];k>0;k--)
        {
            printf("\t");
        }
        printf("%s,%d,%ld,%d,%d,%d,%ld\n", buffer[i].comm, buffer[i].pid, buffer[i].state,
           buffer[i].parent_pid, buffer[i].first_child_pid, buffer[i].next_sibling_pid, buffer[i].uid);
    }
    free(depth);
}

int main(int argc, char **argv)
{
    struct prinfo *buffer=(struct prinfo*)malloc(BUFFER*sizeof(struct prinfo));
    int nr;

    if(syscall(356,buffer,&nr)!=0)
    {
        printf("%s","opps, something bad happened, and please contact Zhicun Chen for help.");
        return -1;
    }
    
    print_pstree(buffer,nr);
    free(buffer);
    return 0;
}