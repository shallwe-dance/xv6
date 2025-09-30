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
#define MMAPSIZE 8192
#define PROT_READ 0x1
void test_filemap_populate() {
  int fd = open("README", O_RDONLY);
  if (fd < 0) {
    printf(1, "[file+populate] failed to open README\n");
    return;
  }

  char *addr = (char *)mmap(0, MMAPSIZE, PROT_READ, MAP_POPULATE, fd, 0);
  printf(1, "[file+populate] mmap returned: %p\n", addr);
  printf(1, "[file+populate] read: %c %c\n", addr[0], addr[1]);
  munmap((uint)addr);
  close(fd);
}

void test_filemap_lazy() {
  int fd = open("README", O_RDONLY);
  if (fd < 0) {
    printf(1, "[file+lazy] failed to open README\n");
    return;
  }

  char *addr = (char *)mmap(0, MMAPSIZE, PROT_READ, 0, fd, 0);
  printf(1, "[file+lazy] mmap returned: %p\n", addr);
  // Should trigger page fault
  printf(1, "[file+lazy] read: %c %c\n", addr[0], addr[1]);
  munmap((uint)addr);
  close(fd);
}

void test_filemap_read_protection_violation() {
  int fd = open("README", O_WRONLY);
  if (fd < 0) {
    printf(1, "[file+prot] failed to open README in O_WRONLY mode\n");
    return;
  }

  char *addr = (char *)mmap(0, MMAPSIZE, PROT_READ, MAP_POPULATE, fd, 0);
  if (addr == 0)
    printf(1, "[file+prot] mmap failed as expected (no read permission)\n");
  else
    printf(1, "[file+prot] mmap unexpectedly succeeded!\n");

  close(fd);
}

int main() {
  printf(1, "--- File-backed mmap() tests ---\n");
  //test_filemap_populate();
  //test_filemap_lazy();
  //test_filemap_read_protection_violation();

  int size = 8192;
int fd = open("README", O_RDONLY);
printf(1,"freemem now is %d\n",freemem());
char* text = (char*)mmap(0, size, PROT_READ, 0, fd, 0);
printf(1, "mmap return is: %d\n", text);
printf(1,"freemem now is %d\n",freemem());
printf(1, "text[4099] is: %c\n", text[4099]);
printf(1,"freemem now is %d\n",freemem());
// for(int i=4096; i<size; i++) printf(1, "%c", *(text+i));
printf(1, "text[300] is: %c\n", text[300]);
printf(1,"freemem now is %d\n",freemem());
printf(1,"\n");
  exit();
}
