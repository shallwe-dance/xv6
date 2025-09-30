
#include "types.h"
#include "stat.h"
#include "user.h"

#define NUM_PAGES  10000   // 총 페이지 수; 물리 메모리보다 크게 설정
#define PGSIZE   4096

int
main(void)
{
  char **ptrs;
  int i;
  int half;

  // 1) ptrs 배열 할당 (스택 오버플로우 방지)
  ptrs = malloc(NUM_PAGES * sizeof(char*));
  if (ptrs == 0) {
    printf(2, "malloc failed\n");
    exit();
  }

  printf(1, "Starting swap-out seeding (alloc + write)...\n");

  // 2) sbrk + 첫 쓰기로 swap-out 유도
  for (i = 0; i < NUM_PAGES; i++) {
    if (i % 1000 == 0)
      printf(1, "  allocating page %d\n", i);
    ptrs[i] = sbrk(PGSIZE);
    if (ptrs[i] == (char*)-1) {
      printf(2, "sbrk failed at page %d\n", i);
      break;
    }
    // 페이지를 실제로 할당받도록 0 쓰기
    ptrs[i][0] = i & 0xFF;
  }

  half = NUM_PAGES / 2;
  printf(1, "Done. First %d pages should now be on swap.\n", half);

  // 3) fork
  int pid = fork();
  if (pid < 0) {
    printf(2, "fork failed\n");
    exit();
  }
  printf(1, "Fork Done\n");

  if (pid == 0) {
    // --- Child: 앞쪽 절반 페이지 읽어서 swap-in 유도 & 검증 ---
    printf(1, "Child: triggering swap-in by reading first %d pages...\n", half);
    for (i = 0; i < half; i++) {
      volatile unsigned char c = ptrs[i][0];
      if (c != (i & 0xFF) && i % 1000 == 0)
        // 너무 많으면 로그가 지저분하니 1000단위로만 찍어봄
        printf(2, "data mismatch at %d: got %d expected %d\n",
               i, c, i & 0xFF);
    }
    printf(1, "Child: swap-in test complete.\n");
    exit();
  } else {
    wait();
    printf(1, "Parent: child finished. Exiting.\n");
    exit();
  }
}
