
#include <iostream>
#include <cstdint>
#include <vector>

#include "mips_memory.hpp"


// constructor:   initialises the memory:

mips_memory::mips_memory(){

    ADDR_INSTR.resize(0x1000000);

    ADDR_DATA.resize(0x4000000);

    //once we got flags and stuff we can add them here to initialise the value if needed
}

void mips_memory::copy_ADDR_INSTR(char* source){

    for (int i = 0; i < INSTR_SIZE; i++){
    
    // TEST //// std::cerr << "the " << i << "th char is" << source[i] << std::endl;
    ADDR_INSTR[i] = source[i]; 
    
    //each element is a char which corresponds to a byte
    }    

    LAST_INSTR_ADDRESS = INSTR_SIZE + 0x10000000 - 4;  

}

void mips_memory::set_INSTR_SIZE(int file_size){

    INSTR_SIZE = file_size;
}

uint32_t mips_memory::read_INSTR(int memory_location){ //the index is the offset memory (to get it from original memory location, subtract 0x10000000)

    uint32_t INSTR;

    int index = memory_location - 0x10000000; //double check the amount

    INSTR = ADDR_INSTR[index];

    INSTR = INSTR << 8 | ADDR_INSTR[index+1];
    INSTR = INSTR << 8 | ADDR_INSTR[index+2];
    INSTR = INSTR << 8 | ADDR_INSTR[index+3];

    return INSTR;   
}

uint32_t mips_memory::read_DATA(int memory_location){ //the index is the offset memory (to get it from original memory location, subtract 0x20000000)

    if((memory_location & 0b11) != 0){ //not alligned
        exit(-11);
    }

    uint32_t DATA;

    if(memory_location < 0x24000000 && memory_location >= 0x20000000){ //if it is in ADDR_DATA area
        int index = memory_location - 0x20000000; // double check the amount

        DATA = ADDR_DATA[index];

        DATA = DATA << 8 | ADDR_DATA[index+1];
        DATA = DATA << 8 | ADDR_DATA[index+2];
        DATA = DATA << 8 | ADDR_DATA[index+3];
    }

    else if(memory_location == 0x30000000){ //GETC

        uint8_t input;
    
        try{
            input = std::getchar();
        }
        catch(std::ios_base::failure){
            //// std::cerr << "input fail" << std::endl;
            std::exit(-21);
        }

        if(std::cin.eof()){
            DATA = 0xFFFFFFFF;  //return -1
        }
        else{
            
            //zero extension
            DATA = input;
        }
    }

    else if(memory_location < 0x11000000 && memory_location >= 0x10000000){ //if it is in ADDR_INSTR

        DATA = read_INSTR(memory_location);
    }
    else{ //address out of bounds

        exit(-11);
    }

    return DATA;  
}

void mips_memory::write_DATA(uint32_t data, int memory_location){//the index is the offset memory (to get it from original memory location, subtract 0x20000000)

    if((memory_location & 0b11) != 0){ //not alligned
        exit(-11);
    }

    if(memory_location < 0x24000000 && memory_location >= 0x20000000){ //if it is in ADDR_DATA area
        int index = memory_location - 0x20000000;

        ADDR_DATA[index + 3] = 0b11111111 & data;
        ADDR_DATA[index + 2] = 0b11111111 & (data >> 8);
        ADDR_DATA[index + 1] = 0b11111111 & (data >> 16);
        ADDR_DATA[index] = 0b11111111 & (data >> 24);
    }

    else if(memory_location == 0x30000004){ //PUTC

        uint8_t temp = data & 0xFF; //truncating the data to a byte

        try{
            //// std::cerr << std::hex << temp << std::endl;
            std::putchar(temp);
        }
        catch(std::ios_base::failure){
        
            // std::cerr << "PUTC error" << std::endl;
            std::exit(-21);
        } 
    } 

    else{ //address out of bounds

        exit(-11);
    } 
}

uint32_t mips_memory::read_LAST_INSTR_ADDRESS(){

    return LAST_INSTR_ADDRESS;
}





/*

For reference (from the spec):

Offset     |  Length     | Name       | R | W | X |
-----------|-------------|------------|---|---|---|--------------------------------------------------------------------
0x00000000 |        0x4  | ADDR_NULL  |   |   | Y | Jumping to this address means the Binary has finished execution.
0x00000004 |  0xFFFFFFC  | ....       |   |   |   |
0x10000000 |  0x1000000  | ADDR_INSTR | Y |   | Y | Executable memory. The Binary should be loaded here.
0x11000000 |  0xF000000  | ....       |   |   |   |
0x20000000 |  0x4000000  | ADDR_DATA  | Y | Y |   | Read-write data area. Should be zero-initialised.
0x24000000 |  0xC000000  | ....       |   |   |   |
0x30000000 |        0x4  | ADDR_GETC  | Y |   |   | Location of memory mapped input. Read-only.
0x30000004 |        0x4  | ADDR_PUTC  |   | Y |   | Location of memory mapped output. Write-only.
0x30000008 | 0xCFFFFFF8  | ....       |   |   |   |
-----------|-------------|------------|---|---|---|-------------------------------------------------------------------- */
