#include "types.h"
#include "user.h"
#include "stat.h"
 void running_wait(void) {
    volatile unsigned long foo=0;
    volatile unsigned long foo2=0;
    for (int i=0;i<200000000;i++) {
        foo++;
        for (int j=0;j<20;j++) {
            foo2++;
        }
        foo2=0;
    }
    }
int main() {
/*
    printf(1, "This is for debugging\n");
    printf(1, "getnice value of pid 1: %d\n",getnice(1));
    for (int i=-1;i<3;i++) {
        printf(1, "return value of getnice(%d) : %d\n", i, getnice(i));
    }
    printf(1, "return value of getnice(99999) : %d\n", getnice(99999));
    
    printf(1, "setnice(-1, 19), return : %d\n", setnice(-1,19));
    printf(1, "getnice(-1), return : %d\n", getnice(-1)); //expected : -1
*/
    setnice(1,5);
    setnice(2,5);
    int pid=fork();
    if (pid==0) {
        setnice(getpid(),0);
        for (volatile unsigned int i=0;i<4200000000; i++) {
            asm volatile("");
        }
        
        ps(0);
        exit();
    } else {
        setnice(getpid(),5);
        for (volatile int k=0;k<1000000000;k++) {
            asm volatile("");
        }
        wait();
       
    }
    exit();
}
