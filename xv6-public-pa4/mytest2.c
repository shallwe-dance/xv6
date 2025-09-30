// mytest_swapin.c
#include "types.h"
#include "stat.h"
#include "user.h"

#define NUM_PAGES 10000
#define PGSIZE     4096

int
main(void)
{
  char **ptrs;
  int i;
  int brd, bwr;      // before read/write
  int ard, awr;      // after  read/write

  // 0) 측정 시작 전 카운터 읽기
  swapstat(&brd, &bwr);

  // 1) ptrs 배열 할당 (스택 오버플로우 방지)
  ptrs = malloc(NUM_PAGES * sizeof(char*));
  if (ptrs == 0) {
    printf(2, "malloc failed\n");
    exit();
  }

  printf(1, "Starting memory allocation test...\n");

  // 2) sbrk + 첫 쓰기로 swap‐out 유도
  for (i = 0; i < NUM_PAGES; i++) {
    if (i % 1000 == 0)
      printf(1, "allocating, iteration %d\n", i);
    ptrs[i] = sbrk(PGSIZE);
    if (ptrs[i] == (char*)-1) {
      printf(2, "sbrk failed at page %d\n", i);
      break;
    }
    // 페이지를 실제로 할당받도록 0 쓰기
    ptrs[i][0] = i & 0xFF;
  }

  // 3) 첫 절반 페이지만 다시 읽어서 swap‐in 유도
  printf(1, "Now reading back first half (%d pages) to trigger swap-in...\n", i/2);
  for (int j = 0; j < i/2; j++) {
    volatile unsigned char c = ptrs[j][0];
    // 데이터 무결성(optional) 확인
    if (c != (j & 0xFF) && j%1000==0)
      printf(2, "data mismatch at %d: got %d, expected %d\n", j, c, j&0xFF);
  }

  // 4) 측정 종료 후 카운터 읽기
  swapstat(&ard, &awr);

  // 5) 차이를 계산해서 출력
  printf(1, "\n=== swap statistics ===\n");
  printf(1, "swap-in  (reads):  %d\n",  ard - brd);
  printf(1, "swap-out (writes): %d\n",  awr - bwr);
  printf(1, "========================\n");

  printf(1, "Test complete. Press any key to exit...\n");
  exit();
}
