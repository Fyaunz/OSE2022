#include "lib/types.h"
#include "lib/riscv.h"
#include "lib/hardware.h"

extern int main(void);
extern void ex(void);
extern void printstring(char *s);

__attribute__((aligned(16))) char kernelstack[4096];
//__attribute__ ((aligned (16))) char userstack[4096];

void printhex(uint64);
void putachar(char);
void copyprog(int);

volatile struct uart *uart0 = (volatile struct uart *)0x10000000;
PCB pcb[2];

int current_process = 0;

// our syscalls
void printstring(char *s)
{
  while (*s)
  {               // as long as the character is not null
    putachar(*s); // output the character
    s++;          // and progress to the next character
  }
}

void putachar(char c)
{
  while ((uart0->LSR & (1 << 5)) == 0)
    ;             // do nothing - wait until bit 5 of LSR = 1
  uart0->THR = c; // then write the character
}

char readachar(void)
{
  while ((uart0->LSR & (1 << 0)) == 0)
    ;                // do nothing - wait until bit 5 of LSR = 1
  return uart0->RBR; // then read the character
}

// just a helper function
void printhex(uint64 x)
{
  int i;
  char s[2];
  s[1] = 0;

  printstring("0x");
  for (i = 60; i >= 0; i -= 4)
  {
    int d = ((x >> i) & 0x0f);
    if (d < 10)
      s[0] = d + '0';
    else
      s[0] = d - 10 + 'a';
    printstring(s);
  }
  printstring("\n");
}

// This is the C code part of the exception handler
// "exception" is called from the assembler function "ex" in ex.S with registers saved on the stack
void exception(stackframe* s)
{
  uint64 nr;
  uint64 param;
  uint64 retval = 0;

  // copy registers a0 and a1 to variables nr and param
  nr = s->a7;
  param = s->a0;
  uint64 pc = r_mepc();

  if ((r_mcause() & (1ULL << 63)) != 0)
  {
    printstring("Timer\n");
    nr = 23;
    *(volatile uint64 *)CLINT_MTIMECMP = *(volatile uint64 *)CLINT_MTIMECMP + 1000000;
    pc = pc -4;
  } else if (r_mcause() != 8)
    {
      printstring("mcause:");
      printhex(r_mcause());
      printstring("mepc:");
      printhex(r_mepc());
      printstring("mtval:");
      printhex(r_mtval());
      return -1;
    }

  switch (nr)
  {
  case 1:
    printstring((char *)param);
    break;
  case 2:
    putachar((char)param);
    break;
  case 3:
    retval = readachar();
    break;
  case 23:
    pcb[current_process].sp = s;
    pcb[current_process].pc = pc;

    printhex(s);
    printhex(pc);

    current_process++;
    if (current_process > 1)
      current_process = 0;

    pc = pcb[current_process].pc;
    s = pcb[current_process].sp;

    printhex(s);
    printhex(pc);
    break;
  case 42: 

    if (current_process == 0)
    {
      pc = (uint64)0x80100000 - 4;
    }
    else
    {
      pc = (uint64)0x80200000 - 4;
    }

    break;
  default:
    printstring("Invalid syscall: ");
    printhex(nr);
    printstring("\n");
    break;
  }

  // adjust return value - we want to return to the instruction _after_ the ecall! (at address mepc+4)
    w_mepc(pc + 4);
  
  // pass the return value back in a0
  asm volatile("mv a0, %0"
               :
               : "r"(retval));
  asm volatile("mv a1, %0"
               :
               : "r"(s));

  // this function returns to ex.S
  // printstring("Exit exeption\n");
}
