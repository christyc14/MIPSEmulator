#include <string>
#include <fstream>
#include <iostream>
#include <vector>

#include <bitset>   //for testing, remove at the end

#include "mips_memory.hpp"
#include "mips_registers.hpp"
#include "mips_breakdown.hpp"


int main(int argc, char *argv[]){ // argc stands for argument count, argv is a one-dimensional array of strings, each containing one of the arguments that was passed to the program.


    if(argc <= 1){ //if there was no bin file passed to the simulator
        //std::cerr << "Error: no file passed to the program" << std::endl;
        exit(-20);
    }


    mips_memory memory; //creating the memory object. It uses vectors of the correct size, filled with 0s

    mips_registers registers;


    ///////////////////////////////////////
    ////////////  Loading File ////////////
    ///////////////////////////////////////

    std::string binLocation = argv[1]; //getting the location of the bin file from the argument

   

    std::ifstream file(binLocation, std::ios::binary | std::ios::ate ); //loads the file, the "ate" flag makes the pointer point to the end of the file, so tellg() shows the size of the file



    if(file.is_open()){ 
        


        std::streampos file_size = file.tellg(); //shows the size of the file
        file.seekg(0, std::ios::beg); // go back to the begining of the file (as we opened with the pointer at the end.)


        if(file_size > 0x1000000){ //if the file size is too big

            exit(-20);
        }

        char *buffer = new char [file_size]; //allocate a temporary buffer to read the data
        memory.set_INSTR_SIZE(file_size); //sets the memory size in bytes so we don't have to work it out again.

        file.read (buffer, file_size);

        file.close();


        memory.copy_ADDR_INSTR(buffer);

        delete[] buffer;  //cleaning up
    }

    else{
        //unable to open file, throw the corresponding error.

        //std::cerr << "Unable to open file" << std::endl;
        exit(-20);
    }

    
    while(1){

        //PC gets set -> the instruction from PC becomes IR -> instruction executes (and sets the next PC)

        if(registers.read_pc() < 0x10000000 || registers.read_pc() > memory.read_LAST_INSTR_ADDRESS()){ //if it is out of range or reached the of of file wihtout going to the address 
            
            // std::cerr << "File ended or PC out of range" << std::endl;
           
            exit(-11);  

        }

        //decode the instruction pointed at by the PC, and execute it. the PC is increased in the function, as they take account of branches etc.
        instruction_decode_run(memory.read_INSTR(registers.read_pc()), memory, registers);

        if(registers.read_reg(0) != 0){
            registers.write_reg(0, 0);
        }
    }
    

    return 1;
}



