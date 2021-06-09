#ifndef MIPS_REGISTERS
#define MIPS_REGISTERS

#include <vector>

class mips_registers{ // mips has 32 registers

public:

  //constructor
  mips_registers();

  //functions for hi and lo

  uint32_t read_hi();
  uint32_t read_lo();

  void write_hi(uint32_t input);
  void write_lo(uint32_t input);

  //things to deal with pc

  uint32_t read_pc();

  //ok so here we have 2 instructions for the program counter, one that just adds 4 for next instruction
  //but the other one is one in case of branches and jumps

  void next_instruction_normal();
  void next_instruction_branch(uint32_t next);

  //generic read and write for all registers
  uint32_t read_reg(const uint8_t& index) const; //corresponds to register number, index in array
  void write_reg(uint8_t index, uint32_t data);


private:

  std::vector<uint32_t> registers;

  uint32_t PC; //Program counter. Starts out with 0x10000000 and shows the absolute location in memory
  
  uint32_t HI; //high and low hold or accumulate the results of multiplication and addition
  uint32_t LO;
};
#endif
