#include "lib/userprogs1.h"
#include "lib/userprogs2.h"
#include "lib/types.h"
#include "lib/riscv.h"
#include "lib/hardware.h"

extern int main(void);
extern void ex(void);
extern void printstring(char *);
extern void printhex(uint64);
extern PCB pcb[];

void copyprog(int process, uint64 address) {
  // copy user code to memory inefficiently... :)
  unsigned char* from;
  int user_bin_len;
  switch (process) {
    case 0: from = (unsigned char *)&user1_bin; user_bin_len = user1_bin_len; break;
    case 1: from = (unsigned char *)&user2_bin; user_bin_len = user2_bin_len; break;
    default: printstring("unknown process!\n"); printhex(process); printstring("\n"); break;
  }

  unsigned char* to   = (unsigned char *)address;
  for (int i=0; i<user_bin_len; i++) {
    *to++ = *from++;
  }
}

void timerinit(void){

  int interval = 1000000;

  *(volatile uint64*)CLINT_MTIMECMP = *(volatile uint64*)CLINT_MTIME + interval;

  w_mstatus(r_mstatus() | MSTATUS_MIE);
  w_mie(r_mie() | MIE_MTIE);

}

void setup(void) {
  // set M Previous Privilege mode to User so mret returns to user mode.
  unsigned long x = r_mstatus();
  x &= ~MSTATUS_MPP_MASK;
  x |= MSTATUS_MPP_U;
  w_mstatus(x);

  // enable machine-mode interrupts.
  w_mstatus(r_mstatus() | MSTATUS_MIE);

  // enable software interrupts (ecall) in M mode.
  w_mie(r_mie() | MIE_MSIE);

  // set the machine-mode trap handler to jump to function "ex" when a trap occurs.
  w_mtvec((uint64)ex);

  // disable paging for now.
  w_satp(0);

  // configure Physical Memory Protection to give user mode access to all of physical memory.
  w_pmpaddr0(0x80000000ull >> 2);
  w_pmpcfg0(0x0f0000);
  w_pmpaddr1(0x80100000ull >> 2);
  //w_pmpcfg1(0x0); 
  w_pmpaddr2(0xffffffffull >> 2);
  //w_pmpcfg2(0xf);            // full access
  
  copyprog(0, 0x80100000);
  copyprog(1, 0x80200000);
  
  pcb[0].pc = 0x80100000;
  pcb[0].sp = 0x80102000;
  pcb[1].pc = 0x80200000;
  pcb[1].sp = 0x80202000;

  // set M Exception Program Counter to main, for mret, requires gcc -mcmodel=medany
  w_mepc((uint64)0x80100000);

  timerinit();

  // switch to user mode (configured in mstatus) and jump to address in mepc CSR -> main().
  asm volatile("mret");
}

