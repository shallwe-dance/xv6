#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"

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
    mycpu()->ticks_save=ticks;
    yield();
    }
    }
    }
  //cprintf("no\n");

  // Check if the process has been killed since we yielded
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    {exit();}

}
