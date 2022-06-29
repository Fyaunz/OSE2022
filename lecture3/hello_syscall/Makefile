SRC_S	= boot.S ex.S
SRC_C	= setup.c kernel.c user.c
LD		= hello.ld
OUT		= main


all: build

build:

	riscv64-unknown-elf-gcc -g -mcmodel=medany -mno-relax -c $(SRC_S)
	riscv64-unknown-elf-gcc -g -mcmodel=medany -mno-relax -c $(SRC_C)
	riscv64-unknown-elf-gcc -g -ffreestanding -fno-common -nostdlib -mno-relax -mcmodel=medany   -Wl,-T $(LD) *.o -o $(OUT)
	riscv64-unknown-elf-objcopy -O binary $(OUT) $(OUT).bin

clean:
	rm -f *.o *.bin

run:
	qemu-system-riscv64 -nographic -machine virt -smp 1 -bios none -kernel ./$(OUT)

rundbg:
	qemu-system-riscv64 -nographic -machine virt -smp 1 -bios none -kernel ./$(OUT) -s -S

startdbg:
	riscv64-unknown-elf-gdb $(OUT)
