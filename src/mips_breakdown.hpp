#include <cstdint>
#include <iostream>
#include <vector>

#include <bitset> //just for testing

#include "mips_memory.hpp"
#include "mips_registers.hpp"

#ifndef MIPS_BREAKDOWN
#define MIPS_BREAKDOWN

//splits the instruction into components and then calls the respective follow up function which executes it
void instruction_decode_run(uint32_t input, mips_memory& memory, mips_registers& registers); 

//executes an R type instruction FINISH IMPLEMENTING
void rtype(uint8_t rs, uint8_t rt, uint8_t rd, uint8_t shamt, uint8_t funct, mips_memory& memory, mips_registers& registers);

//executes an I type instruction IMPLEMENT
void itype(uint8_t opcode, uint8_t rs, uint8_t rt, uint16_t immediate, mips_memory& memory, mips_registers& registers);

//executes an J type instruction IMPLEMENT
void jtype(uint8_t opcode, uint32_t short_address, mips_memory& memory, mips_registers& registers);

#endif



