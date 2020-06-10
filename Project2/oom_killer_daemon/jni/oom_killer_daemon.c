#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(){
    daemon(0,0);
    int i=0;
    
    while(1){
        sleep(10);
        printf("the %d th run oom killer.\n", i++);
        if (syscall(382) != 0){
            printf("oops, oom killer crashed, please rerun this daemon.");
            break;
        }
    }

    return -1;
}