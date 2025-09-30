#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "memlayout.h"
#include "mmu.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "proc.h"
#include "syscall.h"

int
main()
{
    static char buf[512];
    int fd, blocks;

    fd = open("big.file", O_CREATE | O_WRONLY);

    if (fd < 0) {
        printf(1, "bigfile: cannnot open big.file for writing\n");
        exit();
    }

    blocks = 0;
    while (1) {
        *(int *)buf = blocks;
        int cc = write(fd, buf, sizeof(buf));
        if (cc <= 0)
            break;
        blocks++;
        if (blocks % 1000 == 0)
            printf(1, "current blocks : %d\n",blocks);
    }

    printf(1, "\nwrote %d blocks\n", blocks);
    if (blocks != 16523) {
        printf(1, "bigfile: file is too small\n");
        exit();
    }

    close(fd);
    fd=open("big.file", O_RDONLY);

    printf(1, "succeeded to re-open big.file\n");

    int blocks_read=0;
    while (1) {
        int cc=read(fd, buf, sizeof(buf));
        if (cc==0) {break;}//EOF
        if (cc < 0) {
            printf(1, "bigfile: read error\n");
            close(fd);
            exit();
        }
        blocks_read++;
    }
    if (blocks_read != 16523) {
        printf(1, "bigfile: read fail, read %d blocks only\n", blocks_read);
        }
    else {printf(1, "read success!!!! :)\n");}
    close(fd);
    exit();
}
