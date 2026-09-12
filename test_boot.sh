gcc -m32 -ffreestanding -fno-pie -c src/kernel.c -o kernel.o
gcc -m32 -c src/boot.s -o boot.o
ld -m elf_i386 -T linker.ld -o kernel.bin boot.o kernel.o
cp kernel.bin iso/boot/kernel.bin
cp grub.cfg iso/boot/grub/grub.cfg
grub-mkrescue -o os.iso iso
qemu-system-x86_64 -cdrom os.iso
