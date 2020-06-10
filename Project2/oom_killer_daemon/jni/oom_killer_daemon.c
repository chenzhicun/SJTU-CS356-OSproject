#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(){
    daemon(0,0);
    
    while(1){
        // this parameter can control how long the syscall will be called once.
        sleep(10);
        if (syscall(382) != 0){
            printf("oops, oom killer crashed, please rerun this daemon.");
            break;
        }
    }

    return -1;
}