SERIAL?=/dev/ttyUSB0
all:
	patmos-clang -O2 *.c -o VGA.elf


download:
	patserdow -v $(SERIAL) VGA.elf

clean:
	rm -f *.out *.pcap *.pml *.png *.elf
