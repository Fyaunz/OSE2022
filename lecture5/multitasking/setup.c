#include "userprogs1.h"
#include "userprogs2.h"
#include "types.h"
#include "riscv.h"

extern int main(void);
extern void ex(void);
extern void printstring(char *);
extern void printhex(uint64);

void copyprog(int process) {
  // copy user code to memory inefficiently... :)
  unsigned char* from;
  int user_bin_len;
  switch (process) {
    case 0: from = (unsigned char *)&user1_bin; user_bin_len = user1_bin_len; break;
    case 1: from = (unsigned char *)&user2_bin; user_bin_len = user2_bin_len; break;
    default: printstring("unknown process!\n"); printhex(process); printstring("\n"); break;
  }

  unsigned char* to   = (unsigned char *)0x80100000;
  for (int i=0; i<user_bin_len; i++) {
    *to++ = *from++;
  }
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
  w_pmpaddr0(0x3fffffffffffffull);
  w_pmpcfg0(0xf);            // full access

  // TODO 3.1: Add configuration for PMP so that the I/O and kernel address space is protected

  copyprog(0);

  // TODO 3.2: Add initalization of correct values in the PCB for all processes.

  // set M Exception Program Counter to main, for mret, requires gcc -mcmodel=medany
  w_mepc((uint64)0x80100000);

  // switch to user mode (configured in mstatus) and jump to address in mepc CSR -> main().
  asm volatile("mret");
}

