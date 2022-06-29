#include "lib/types.h"

__attribute__ ((aligned (16))) char userstack[4096];

uint64 syscall(uint64 nr, uint64 param) {
    uint64 retval;
    asm volatile("mv a7, %0" : : "r" (nr));
    asm volatile("mv a0, %0" : : "r" (param));
    asm volatile("ecall");
    asm volatile("mv %0, a0" : "=r" (retval) );
    return retval;
}

volatile int main(void) {
    /*syscall(1, "Der Prozess 1 wurde gestartet\n");
    syscall(2, 'a');
    syscall(1, "Es wird nun in Prozess 2 übergegangen\n");
    syscall(23, 0);
    syscall(1, "Der Prozess 1 wird nun beendet\n");*/
    while (1)
    {
        syscall(2, 'a');
        syscall(2, 'b');
        //syscall(23, 0);
    }
    
}
