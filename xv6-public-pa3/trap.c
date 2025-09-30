#include "types.h"
#include "stat.h"
#include "defs.h"
#include "fcntl.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;

const int weight[40]= {
/* 0*/  88761,  71755,  56483,  46273,  36291,
/* 5*/  29154,  23254,  18705,  14949,  11916,  
/*10*/   9548,   7620,   6100,   4904,   3906,
/*15*/   3121,   2501,   1991,   1586,   1277,
/*20*/   1024,    820,    655,    526,    423,
/*25*/    335,    272,    215,    172,    137,
/*30*/    110,     87,     70,     56,     45,
/*35*/     36,     29,     23,     18,     15
};

void
tvinit(void)
{
  int i;

  for(i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);

  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  lidt(idt, sizeof(idt));
}

//trapframe contains the processor register
//PAGEBREAK: 41

int page_fault_handler(struct trapframe* tf) {
    //cprintf("handler called\n");
    uint fault_address=rcr2();
    int if_write = (tf->err&2)!=0 ? 1 : 0;

    struct mmap_area* according_mapping_region=0;
    for (int i=0;i<64;i++) {
        uint start_address=MMAPBASE+(mmap_area_array[i].addr);
        uint end_address=start_address+(mmap_area_array[i].length);
        if (fault_address>=start_address && fault_address < end_address) {
            according_mapping_region=&mmap_area_array[i];
        }
    } if (!according_mapping_region) {
        return -1;
    } if (according_mapping_region->prot==PROT_READ && if_write) {
        return -1;
    }
    uint according_page_start=PGROUNDDOWN(fault_address);
    char* physical_mem=kalloc();
    if (!physical_mem) {return -1;}
    memset(physical_mem,0,4096);
    if (according_mapping_region->flags < MAP_ANONYMOUS) {
        uint region_base = MMAPBASE + according_mapping_region->addr;
uint file_offset = according_mapping_region->offset + (according_page_start - region_base);
according_mapping_region->f->off = file_offset;
        //according_mapping_region->f->off=according_mapping_region->offset;
        int fr = fileread(according_mapping_region->f, (char*)physical_mem, 4096);
        //according_mapping_region->f->off+=4096;
        if (fr<0) {cprintf("fileread failed\n");}
        
    }
    int perm= PTE_U | PTE_P;
    if (according_mapping_region->prot & PROT_WRITE) {perm |= PTE_W;}
    mappages((according_mapping_region->p)->pgdir, (void*)according_page_start, 4096, V2P(physical_mem), perm);

    return 0;
    }
void
trap(struct trapframe *tf)
{
  //cprintf("trap called, trapno : %d\n",tf->trapno);
  if(tf->trapno == T_SYSCALL){
    if(myproc()->killed)
      exit();
    myproc()->tf = tf;
    syscall();
    if(myproc()->killed)
      exit();
    return;
  }

  //only for page fault
  if (tf->trapno==T_PGFLT) {

  //***************************************************
  
  if (page_fault_handler(tf)==0) {return;}
  else {
    //cprintf("Something wrong with page fault handler\n");
  exit();}
  
  //*******************************************************

  /*
    //cprintf("page fault !\n");
    uint fault_address=rcr2();
    int if_write = (tf->err&2)!=0 ? 1 : 0;

    //find according mapping region
    struct mmap_area* according_mapping_region=0;
    for (int i=0;i<64;i++) {
        uint start_address=MMAPBASE+(mmap_area_array[i].addr);
        uint end_address=start_address+(mmap_area_array[i].length);
        if (fault_address>=start_address && fault_address<end_address) {
            according_mapping_region=&mmap_area_array[i];
        }
    } if (!according_mapping_region) {
        exit();
        return;
        }
    if (according_mapping_region->prot==PROT_READ && if_write) {
        exit();
        return;
    }

    uint according_page_start=MMAPBASE;
    //for only one page according to faulted address
    according_page_start=PGROUNDDOWN(fault_address);

    //allocate new physical page
    char *physical_mem=kalloc();

    //fill with 0
    memset(physical_mem,0,4096);

    //if file mapping, read file into physical memory
    //should consider offset!!!
    if (according_mapping_region->flags<MAP_ANONYMOUS) {
    //cprintf("page fault handled for file mapping\n");
    uint region_base=according_mapping_region->addr + MMAPBASE;
    uint file_offset=according_mapping_region->offset + (according_page_start-region_base);
    according_mapping_region->f->off=file_offset;
    fileread(according_mapping_region->f, physical_mem,4096);
    }

    //if anonymous, just leave the page

    //make page table, make it properly(ex. set PTE_W value)
    int perm= PTE_U | PTE_P;
    if (according_mapping_region->prot & PROT_WRITE) {perm |= PTE_W;}
    mappages((according_mapping_region->p)->pgdir, (void*)according_page_start, 4096, V2P(physical_mem), perm);
    return;
    */
  }


  //looks trapno to decide why it has been called and what needs to be done
  switch(tf->trapno){

  case T_IRQ0 + IRQ_TIMER://When timeout!
    //cprintf("timeout\n");
    //todo:update runtime, vruntime
    if(cpuid() == 0){
      acquire(&tickslock);
      ticks++;//1. increment tick
      //cprintf("incremented ticks\n");
      //edited : update runtime/vruntime
      if (myproc()) {
      myproc()->runtime+=1000;
      //cprintf("updated runtime\n");
      myproc()->vruntime+=(int)(1000*1024/weight[myproc()->nice]);
      //cprintf("updated vruntime also\n");
      }
      wakeup(&ticks);//2. wakeup
      release(&tickslock);
    }
    lapiceoi();//ack-ing interrupt
    break;
  case T_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE+1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case T_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpuid(), tf->cs, tf->eip);
    lapiceoi();
    break;

  //PAGEBREAK: 13
  default:
    if(myproc() == 0 || (tf->cs&3) == 0){
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpuid(), tf->eip, rcr2());
      panic("trap");
    }
    // In user space, assume process misbehaved.
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            myproc()->pid, myproc()->name, tf->trapno,
            tf->err, cpuid(), tf->eip, rcr2());
    myproc()->killed = 1;
  }

  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running
  // until it gets to the regular system call return.)
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  //if (myproc()) {cprintf("ticks_save : %d, weight : %d, ticks : %d, weight_sum : %d\n",mycpu()->ticks_save, weight[myproc()->nice], ticks, calculate_weight_sum());}
  if(myproc() && myproc()->state == RUNNING && tf->trapno == T_IRQ0+IRQ_TIMER) { //timeout
    //cprintf("check if time slice finished ->");
    //need to check if RUNNABLE process exists
    int weight_sum=calculate_weight_sum();
    if (weight_sum){
    if (mycpu()->ticks_save+((int)(10*weight[myproc()->nice]/weight_sum))<ticks) {
    //cprintf("yes\n");
    mycpu()->ticks_save+=ticks;
    yield();
    }
    }
    }
  //cprintf("no\n");

  // Check if the process has been killed since we yielded
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    {exit();}

}
