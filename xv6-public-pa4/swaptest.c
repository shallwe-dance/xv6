#include "param.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"
#include "memlayout.h"


int main () {
	int a, b;

    swapstat(&a, &b);
    printf(1, "swap sectors read : %d\n", a);
    printf(1, "swap sectors write: %d\n", b);
    exit();
}
