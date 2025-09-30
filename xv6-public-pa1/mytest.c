#include "types.h"
#include "user.h"
#include "stat.h"

int main() {
    printf(1, "This is for debugging\n");
    printf(1, "getnice value of pid 1: %d\n",getnice(1));
    for (int i=-1;i<3;i++) {
        printf(1, "return value of getnice(%d) : %d\n", i, getnice(i));
    }
    printf(1, "return value of getnice(99999) : %d\n", getnice(99999));
    
    printf(1, "setnice(-1, 19), return : %d\n", setnice(-1,19));
    printf(1, "getnice(-1), return : %d\n", getnice(-1)); //expected : -1

    printf(1, "setnice(1, 19), return : %d\n",setnice(1,19));
    printf(1, "getnice value of pid 1 : %d\n",getnice(1));
    ps(0);
    exit();
}
