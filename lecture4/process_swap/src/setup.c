#include "lib/userprogs1.h"
#include "lib/userprogs2.h"
#include "lib/types.h"
#include "lib/riscv.h"

extern int main(void);
extern void ex(void);
extern void printstring(char *);
extern void printhex(uint64);

void copyprog(int process) {
  // TODO: copy user code to memory at address 0x8010_0000
  switch (process) {
    case 0:
      uint8_t* pointer0 = 0x80100000;
      for(int i = 0;i<user1_bin_len;i++){
        *pointer0 = user1_bin[i];
        *pointer0++;
      }
      break;
    case 1:
      uint8_t* pointer1 = 0x80100000;
      for(int i = 0;i<user2_bin_len;i++){
        *pointer1 = user2_bin[i];
        *pointer1++;
      }
      break;
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
  w_pmpcfg0(0xf);

  // copy the first process into memory
  copyprog(0);

  // set M Exception Program Counter to main, for mret, requires gcc -mcmodel=medany
  w_mepc((uint64)0x80100000);

  // switch to user mode (configured in mstatus) and jump to address in mepc CSR -> main().
  asm volatile("mret");
}

