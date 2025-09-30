#include "types.h"
#include "stat.h"
#include "user.h"

#define NUM_PAGES 30000   // 페이지 수 (조절 가능)
#define PGSIZE 4096

int
main(void)
{
  char *ptrs[NUM_PAGES];

  printf(1, "Starting memory allocation test...\n");

  for (int i = 0; i < NUM_PAGES; i++) {
    if (i%1000==0) {printf(1, "allocating, current iteration : %d\n",i);}
    ptrs[i] = sbrk(PGSIZE);
    if (ptrs[i] == (char*)-1) {
      printf(1, "sbrk failed at page %d\n", i);
      break;
    }
    // 페이지를 실제로 할당받도록 0을 씀
    ptrs[i][0] = i % 256;
  }

  printf(1, "Memory allocation complete. Press any key to exit...\n");

  exit();
}


