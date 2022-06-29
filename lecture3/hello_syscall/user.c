#include "types.h"

uint64 syscall(uint64 nr, uint64 param) {
    uint64 retval;

    asm volatile("add a7, %0, x0" : : "r" (nr));
    asm volatile("add a0, %0, x0" : : "r" (param));

    // here's our ecall!
    asm volatile("ecall");

    asm volatile("add %0, a0, x0 " : "=r" (retval));

    // Here we return the return value...
    return retval;
}

void sysputachar(char c){
  syscall(1, c);
}

char sysgetachar(void){
  syscall(2, 0);
}

void sysprintstring(char *s){
  syscall(0, s);
}

int main(void) {
    char c = 0;
    syscall(0, (uint64)"Hallo Bamberg!\n");
    do {
      c = syscall(2, 0);
      if (c >= 'a' && c <= 'z') c = c & ~0x20;
      syscall(1, c);
    } while (c != 'X');

    syscall(0, (uint64)"This is the end!\n");
    return 0;
}

