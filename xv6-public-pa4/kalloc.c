// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.
#include "types.h"
#include "mmu.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "x86.h"

void freerange(void *vstart, void *vend);
extern char end[]; // first address after kernel loaded from ELF file
                   // defined by the kernel linker script in kernel.ld

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  int use_lock;
  struct run *freelist;
} kmem;
#define BITMAP_USING(bm, i)	((bm[(i) / 8] >> ((i) % 8)) & 1)
#define BITMAP_SET(bm, i)	(bm[(i) / 8] |= (1 << ((i) % 8)))
#define BITMAP_CLR(bm, i)	(bm[(i) / 8] &= ~(1 << ((i) % 8)))

char* swap_bitmap;
void
swapinit(void) {
  swap_bitmap=kalloc();
  memset(swap_bitmap,0,PGSIZE);//이왕 할당받은 거 PGSIZE로 초기화
}

int find_free_swap_slot(void) {
    for (int i=0;i<MAXSWAPSLOT;i++) {
    	if (!BITMAP_USING(swap_bitmap, i)) {
		BITMAP_SET(swap_bitmap, i);
		return i;
	}
    }
    return -1;
}

struct page pages[PHYSTOP/PGSIZE];
struct page *page_lru_head;
int num_free_pages;
int num_lru_pages;



void LRU_list_insert(pde_t* pgdir, char* vaddr) {
    struct page* new_page=0;
    for (int i=0;i<PHYSTOP/PGSIZE;i++) {
        if (pages[i].pgdir==pgdir && pages[i].vaddr==vaddr) {
            new_page=&pages[i];
            break;
        }
    }
    if (!new_page) {panic("page not found for LRU insert");}

    if (!page_lru_head) {
        page_lru_head=new_page;
        new_page->next=new_page;
        new_page->prev=new_page;
    }
    else {
        new_page->next=page_lru_head;
        new_page->prev=page_lru_head->prev;
        new_page->prev->next=new_page;
        page_lru_head->prev=new_page;
        }
    return;
}


void LRU_list_remove(pde_t* pgdir, char* vaddr) {
    if (!page_lru_head) {return;}
    struct page* victim=0;
    struct page *iter = page_lru_head;
    do {
        if (iter->pgdir == pgdir && iter->vaddr == vaddr) {
            victim = iter;
            break;
        }
        iter = iter->next;
    } while (iter != page_lru_head);

    if (!victim) {return;}

    if (victim->next==victim) {
        page_lru_head=0;
    } else {
        victim->prev->next=victim->next;
        victim->next->prev=victim->prev;

        if (page_lru_head==victim) {page_lru_head=victim->next;}
    }
    num_lru_pages--;
    victim->next=0;
    victim->prev=0;
    return;
}


// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
void
kinit1(void *vstart, void *vend)
{
  initlock(&kmem.lock, "kmem");
  kmem.use_lock = 0;
  freerange(vstart, vend);
}

void
kinit2(void *vstart, void *vend)
{
  freerange(vstart, vend);
  kmem.use_lock = 1;
}


void
freerange(void *vstart, void *vend)
{
  char *p;
  p = (char*)PGROUNDUP((uint)vstart);
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
    kfree(p);
}
//PAGEBREAK: 21
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(char *v)
{
  struct run *r;

  if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(v, 1, PGSIZE);

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = (struct run*)v;
  r->next = kmem.freelist;
  kmem.freelist = r;
  if(kmem.use_lock)
    release(&kmem.lock);
}

//Consideration : Use 1 physical page for bitmap to track swap space
//byte to bit
int reclaim(void) {
    //get first page in LRU list
//    int LRU_exist=0;
    while (1) {
        struct page* victim=page_lru_head;
        //if (victim) {cprintf("victim exists\n");}
        //else {cprintf("victime doesn't exist\n");}
        pde_t* pgdir=victim->pgdir;
    	char* vaddr=victim->vaddr;
    	pte_t* pte=walkpgdir(pgdir,vaddr,0);
        if (!pte) {panic("no pte found in reclaim function.");}
	//if PTE_A==1 
	if (*pte & PTE_A) {
	    *pte &= ~PTE_A; //clear PTE_A bit
	    //send the page to the tail of LRU list
	    LRU_list_remove(pgdir,vaddr);
        LRU_list_insert(pgdir, vaddr);
	} else {
	    //if PTE_A==0
	    //find free space in swap space (disk)
	    int swap_slot=find_free_swap_slot();
        if (swap_slot<0) {
            cprintf("!!!!!OOM!!!!!\n");
            return 0;
            }
        //cprintf("free swap slot : %d\n",swap_slot);
	    //extract physical memory address of the mapping
        if (!(*pte & PTE_P)) {panic("reclaim : not present");}
	    uint pa=PTE_ADDR(*pte);
        //cprintf("acquired pa of pte\n");
        //***********************done so far*******************************
        if (!pte) {panic("pte not exists");}
        if (!(*pte & PTE_P)) {panic("pte doesn't exist or PTE_P is 0");}
	    swapwrite((char*)P2V(pa),swap_slot);//write victim to free space in disk
        //cprintf("done swap write\n");
        //cprintf("pgdir when swap out : %x, swap_slot : %d\n",pgdir,swap_slot);

        //remove page from LRU_list
        LRU_list_remove(pgdir, vaddr);

        //remove page from pages[]
        free_page(pgdir, vaddr);
        
	    //free present page
	    kfree(P2V(pa));
	    num_free_pages++;
	    //clear PTE_P
        //int old_flags=PTE_FLAGS(*pte) & (PTE_W|PTE_U);

	    *pte = (swap_slot << 12) | (*pte&(PTE_W|PTE_U)); //this pte is in 'disk'!
	    *pte &= ~PTE_P;//clear P bit
        //ensure A bit is 0
        //TLB flush
        invlpg(vaddr);

        //decrement the number of lru_pages (done in LRU_list_remove)
	    //BITMAP_CLR(swap_bitmap, swap_slot);
	    break;
	}
    }
    //cprintf("done reclaim\n");
    return 1;   
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(void)
{
  struct run *r;

try_again:
  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = kmem.freelist;
  if(!r) {
    if (kmem.use_lock) {release(&kmem.lock);}
    if (reclaim()) {goto try_again;}
    else {
    cprintf("!!!!!OOM!!!!!\n");
    return 0;
    }
    if (kmem.use_lock) {acquire(&kmem.lock);}
      }
  if(r)
    kmem.freelist = r->next;
  if(kmem.use_lock)
    release(&kmem.lock);
  return (char*)r;
}

