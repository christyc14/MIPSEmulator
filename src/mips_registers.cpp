#include <cstdint>
#include "mips_registers.hpp"
#include <iostream>
#include <vector>

//ADD A FUNCTION TO CHECK THAT REGISTER 0 IS ALWAYS 0

//constructor
mips_registers::mips_registers(){ 
  PC = 0x10000000;
  registers.resize(32);
  
  registers[0] = 0;

  //add any other initialization needed
}
//read hi register
uint32_t mips_registers::read_hi(){
  return HI;
}

uint32_t mips_registers::read_lo(){
  return LO;
}

void mips_registers::write_hi(uint32_t input){
  HI = input;
}

void mips_registers::write_lo(uint32_t input){
  LO = input;
}

//things to deal with pc
uint32_t mips_registers::read_pc(){
  return PC;
}

//ok so here we have 2 instructions for the program counter, one that just adds 4 for next instruction
//but the other one is one in case of branches and jumps
void mips_registers::next_instruction_normal(){
  PC = PC + 4;
}

void mips_registers::next_instruction_branch(uint32_t next){
  PC = next;
}

//generic read and write for all registers
uint32_t mips_registers::read_reg(const uint8_t& index) const{
  return registers[index];
}//corresponds to register number, index in array

void mips_registers::write_reg(uint8_t index, uint32_t data){
  if(index != 0){ // because $0 is always 0!!!!
    registers[index] = data;
  }
}
