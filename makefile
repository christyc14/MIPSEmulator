#This is the example makefile provided, adapted to our structure of the code.

# Example usage:
# If you have testbench/my_test.c, you can execute the following to create a MIPS binary
# make testbench/my_test.mips.bin

# For simulator
CC = g++
CPPFLAGS = -std=c++11 -W -Wall 

# For MIPS binaries. Turn on all warnings, enable all optimisations and link everything statically
MIPS_CC = mips-linux-gnu-gcc
MIPS_OBJCOPY = mips-linux-gnu-objcopy
MIPS_OBJDUMP = mips-linux-gnu-objdump
MIPS_CPPFLAGS = -W -Wall -O3 -fno-builtin -march=mips1 -mfp32
MIPS_LDFLAGS = -nostdlib -Wl,-melf32btsmip -march=mips1 -nostartfiles -mno-check-zero-division -Wl,--gpsize=0 -static -Wl,-Bstatic -Wl,--build-id=none

# Compile C file (.c) into MIPS object file (.o)
%.mips.o: %.c
	$(MIPS_CC) $(MIPS_CPPFLAGS) -c $< -o $@

# Assemble MIPS assembly file (.s) into MIPS object file (.o)
%.mips.o: %.s
	$(MIPS_CC) $(MIPS_CPPFLAGS) -c $< -o $@

# Link MIPS object file (.o), producing .elf, using memory locations specified in spec
%.mips.elf: %.mips.o
	$(MIPS_CC) $(MIPS_CPPFLAGS) $(MIPS_LDFLAGS) -T linker.ld $< -o $@

# Extract binary instructions only from linked object file (.elf)
%.mips.bin: %.mips.elf
	$(MIPS_OBJCOPY) -O binary --only-section=.text $< $@

# Disassemble linked object file (.elf), pulling out instructions as MIPS assembly file (.s)
%.mips.s : %.mips.elf
	$(MIPS_OBJDUMP) -j .text -D $< > $@

# Dummy for build simulator to conform to spec
simulator: bin/mips_simulator

# Build simulator
bin/mips_simulator: simulator_main.o mips_memory.o mips_registers.o mips_breakdown.o 
	mkdir -p bin
	$(CC) $(CPPFLAGS) src/simulator_main.o src/mips_memory.o src/mips_breakdown.o src/mips_registers.o  -o bin/mips_simulator  

mips_memory.o: src/mips_memory.cpp src/mips_memory.hpp
	$(CC) $(CPPFLAGS) -c src/mips_memory.cpp -o src/mips_memory.o

mips_registers.o: src/mips_registers.cpp src/mips_registers.hpp
	$(CC) $(CPPFLAGS) -c src/mips_registers.cpp -o src/mips_registers.o

simulator_main.o: src/simulator.cpp
	$(CC) $(CPPFLAGS) -c src/simulator.cpp -o src/simulator_main.o

mips_breakdown.o: src/mips_breakdown.cpp src/mips_breakdown.hpp
	$(CC) $(CPPFLAGS) -c src/mips_breakdown.cpp -o src/mips_breakdown.o


# Dummy for build testbench to conform to spec. Could do nothing
testbench:
	@echo "Nothing to do"