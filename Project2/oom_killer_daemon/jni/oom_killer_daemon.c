/*
 *
 * This file is a daemon program. It will call the oom killer syscall with the
 * given time interval. To run this file, you need first install the oom killer
 * module. The proper format to run this file is like:
 *
 * ./oom_daemonARM ${Time}
 *
 * written by Zhicun Chen
 * 2020/06/10
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv){
    daemon(0,0);
    int sleepTime = atoi(argv[1]);
    printf("The time interval to run the oom killer is %d.\n", sleepTime);

    while(1){
        // this parameter can control how long the syscall will be called once.
        sleep(sleepTime);
        if (syscall(382) != 0){
            printf("oops, oom killer crashed, please rerun this daemon.");
            break;
        }
    }

    return -1;
}