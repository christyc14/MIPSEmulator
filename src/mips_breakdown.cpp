#include "mips_breakdown.hpp"

void instruction_decode_run(uint32_t input, mips_memory& memory, mips_registers& registers){

    
    uint8_t opcode = input >> 26 ;

    // std::cerr << "the instruction is " << std::bitset<32>(input) << std::endl;
    //// std::cerr << "the opcode is " << std::bitset<8>(opcode) << std::endl;

    if (opcode == 0){ // R-type instructions

        //Splitting the instruction into components:

        //rs - 1st register operand (5 bits)
        uint8_t rs = (input >> 21) & 0x1F;

        // std::cerr << "instruction is " << std::hex << input << " rs = " << rs << " contents = " << registers.read_reg(rs) << std::endl;

        //rt - 2nd register operand (5 bits)
        uint8_t rt = (input >> 16) & 0x1F;

        //rd - register destination (5 bits)
        uint8_t rd = (input >> 11) & 0x1F;

        //shamt - shift amount (5 bits)
        uint8_t shamt = (input >> 6) & 0x1F;

        //funct - function code (identifies the specific R type instruction) (6 bits)
        uint8_t funct = input & 0x3F;

        /*

        // std::cerr << "rs " << std::bitset<8>(rs) << std::endl;
        // std::cerr << "rt " << std::bitset<8>(rt) << std::endl;
        // std::cerr << "rd " << std::bitset<8>(rd) << std::endl;
        // std::cerr << "shamt " << std::bitset<8>(shamt) << std::endl;
        // std::cerr << "funct " << std::bitset<8>(funct) << std::endl;

        */

        //execute the instruction:
        return rtype(rs, rt, rd, shamt, funct, memory, registers);

    }

    else if (opcode == 0b001000 || opcode == 0b001001 || opcode == 0b001100 || opcode == 0b000100 || opcode == 0b000001 || opcode == 0b000111 || opcode == 0b000110 || opcode == 0b000101 || opcode == 0b100000 || opcode == 0b100100 || opcode == 0b100001 || opcode == 0b100101 || opcode == 0b001111 || opcode == 0b100011 || opcode == 0b100010 || opcode == 0b100110 || opcode == 0b001101 || opcode == 0b101000 || opcode == 0b001010 || opcode == 0b001011 || opcode == 0b101001 || opcode == 0b101011 || opcode == 0b001110){
        // I type

        //Spliting the instruction into components:

        //rs - register containing base address (5 bits)
        uint8_t rs = (input >> 21) & 0x1F;

        //rt - register destination/source (5 bits)
        uint8_t rt = (input >> 16) & 0x1F;

        //immediate - value or offset (16 bits)
        uint16_t immediate = input & 0xFFFF;

        //execute the instruction:
        return itype(opcode, rs, rt, immediate, memory, registers);
    }

    else if (opcode == 0b000010 || opcode == 0b000011){ // J Type 

        //26-bit shortened address of the destination. 2 LSB are removed (since they would be 0), and 4 MSB are removed and assumed to be same as current address
        uint32_t short_address = input & 0x07FFFFFF;

        //execute the instruction:
        return jtype(opcode, short_address, memory, registers);
    }

    else { //not a valid opcode

        exit(-12);
    }
}

void rtype(uint8_t rs, uint8_t rt, uint8_t rd, uint8_t shamt, uint8_t funct, mips_memory& memory, mips_registers& registers){

    //// std::cerr << "entered r type" << std::endl;
    //ADDU
    if(funct == 0b100001 && shamt == 0){

        // std::cerr << "entered ADDU" << std::endl;

        //get the result of the addition of the values in the two registers
        uint32_t result = registers.read_reg(rs) + registers.read_reg(rt);

        //put the result into the destination register
        registers.write_reg(rd, result);

        //update the program counter
        registers.next_instruction_normal();

        //// std::cerr << "exiting ADDU" << std::endl;
    }

    //JR
    else if(funct == 0b001000 && rt == 0 && rd == 0 && shamt == 0){

        // std::cerr << "entered JR" << std::endl;

        //reads the address for the jump from the rs register:
        uint32_t jump_address = registers.read_reg(rs);

        //if the address from the register is alligned
        if(jump_address % 4 == 0){

            //execute next instruction since there is a branch delay. 
            //it reads the instruction after the PC (which is PC+4), and then puts it into the instruction breakdown function recursively  
            
            // std::cerr << "Current PC: " << registers.read_pc() << ", LAST_INSTR_ADDRESS: " << memory.read_LAST_INSTR_ADDRESS() << std::endl; 

            if(registers.read_pc() < memory.read_LAST_INSTR_ADDRESS()){ //only executes the branch delay instruction if it there is an instruction after the JR

                instruction_decode_run(memory.read_INSTR(registers.read_pc()+4), memory, registers);
            }

            //if the address points at 0x0, which means the program has finished execution, exit and indicate success
            if(jump_address == 0){

                uint8_t exit_code = registers.read_reg(2);

                exit(exit_code);
            }

            //if the address is out of bounds, exit and indicate memory error
            else if(jump_address < 0x10000000 || jump_address >= 0x11000000){
               
                exit(-11);
            }

            //set the PC to the next instruction.
            registers.next_instruction_branch(jump_address);
           
        }
        else{
        
        //address from the register is not alligned
            // std::cerr << "JR address not alligned" << std::endl;
            exit(-11);
        }
    }

    //ADD
    else if(funct == 0b100000 && shamt == 0){

        // std::cerr << "entered ADD" << std::endl;

        int32_t a = registers.read_reg(rs);
        int32_t b = registers.read_reg(rt);
        int32_t sum = a + b;

        // std::cerr << "register rs: " << rs << " contents: " << std::hex << a << std::endl;
        // std::cerr << "register rt: " << rt << " contents: " << std::hex << b << std::endl;
        // std::cerr << "sum: " << std::hex << sum << std::endl;


        //checking for 2s complement overflow
        if(((a >= 0) && (b >= 0) && (sum < 0)) || ((a < 0) && (b < 0) && (sum >= 0))){ //overflow detected

            exit(-10);
        }
        else{

            registers.write_reg(rd, sum);
            
            registers.next_instruction_normal();
        }
    }

    //AND
    else if(funct == 0b100100 && shamt == 0){

        uint32_t result = registers.read_reg(rs) & registers.read_reg(rt);

        registers.write_reg(rd, result);

        registers.next_instruction_normal();
    }

    //DIV
    else if(funct == 0b011010 && rd == 0 && shamt == 0){

        //convert the register values to signed
        int32_t a = registers.read_reg(rs);
        int32_t b = registers.read_reg(rt);

        if(b != 0){

            int32_t quotient = a / b;
            int32_t remainder = a % b;

            registers.write_hi(remainder);
            registers.write_lo(quotient);
        }

        registers.next_instruction_normal();
    }

    //DIVU
    else if(funct == 0b011011 && rd == 0 && shamt == 0){

        //convert the register values to signed
        uint32_t a = registers.read_reg(rs);
        uint32_t b = registers.read_reg(rt);

        if(b != 0){

            uint32_t quotient = a / b;
            uint32_t remainder = a % b;

            registers.write_hi(remainder);
            registers.write_lo(quotient);
        }

        registers.next_instruction_normal();
    }

    //JALR
    else if(funct == 0b001001 && rt == 0 && shamt == 0){

        // std::cerr << "entered JALR" << std::endl;

        uint32_t destination_address = registers.read_reg(rs);

        registers.write_reg(rd, registers.read_pc() + 8);

        if(destination_address % 4 == 0){

            //branch delay
            if(registers.read_pc() < memory.read_LAST_INSTR_ADDRESS()){ //only executes the branch delay instruction if it there is an instruction after

                // std::cerr << "entered Branch Delay in JALR" << std::endl;

                instruction_decode_run(memory.read_INSTR(registers.read_pc()+4), memory, registers);
            }   

            //if the address points at 0x0, which means the program has finished execution, exit and indicate success
            if(destination_address == 0){

                uint8_t exit_code = registers.read_reg(2);

                exit(exit_code);
            }

            //if the address is out of bounds, exit and indicate memory error
            else if(destination_address < 0x10000000 || destination_address >= 0x11000000){
               
                exit(-11);
            }

            registers.next_instruction_branch(destination_address);
        }
        else{

            exit(-11);
        }
    }

    //MFHI
    else if(funct == 0b010000 && rs == 0 && rt == 0 && shamt == 0){

        registers.write_reg(rd, registers.read_hi());

        registers.next_instruction_normal();
    }

    //MFLO
    else if(funct == 0b010010 && rs == 0 && rt == 0 && shamt == 0){

        registers.write_reg(rd, registers.read_lo());

        registers.next_instruction_normal();
    }

    //MTHI
    else if(funct == 0b010001 && rt == 0 && rd == 0 && shamt == 0){

        registers.write_hi(registers.read_reg(rs));

        registers.next_instruction_normal();
    }

    //MTLO
    else if(funct == 0b010011 && rt == 0 && rd == 0 && shamt == 0){

        registers.write_lo(registers.read_reg(rs));

        registers.next_instruction_normal();
    }

    //MULT
    else if(funct == 0b011000 && rd == 0 && shamt == 0){

        // std::cerr << "entered MULT" << std::endl;

        int64_t result;

        //make the registers signed
        int32_t a = registers.read_reg(rs);
        int32_t b = registers.read_reg(rt);

        //sign extending them to 64 bits
        int64_t a_ext = a;
        int64_t b_ext = b;

        

        result = a_ext * b_ext;

        // std::cerr << "rs content: " << a << " rt content: " << b << "result: " << result << std::endl;

        registers.write_lo(result & 0xFFFFFFFF);
        registers.write_hi((result >> 32) & 0xFFFFFFFF);

        // std::cerr << "hi: " << registers.read_hi() << " lo: " << registers.read_lo() << std::endl;
        registers.next_instruction_normal();
    }

    //MULTU
    else if(funct == 0b011001 && rd == 0 && shamt == 0){

        uint64_t result;

        uint64_t a_ext = registers.read_reg(rs);
        uint64_t b_ext = registers.read_reg(rt);

        result = a_ext * b_ext;

        registers.write_lo(result & 0xFFFFFFFF);
        registers.write_hi((result >> 32) & 0xFFFFFFFF);

        registers.next_instruction_normal();
    }

    //OR
    else if(funct == 0b100101 && shamt == 0){
        registers.write_reg(rd, registers.read_reg(rs) | registers.read_reg(rt));

        registers.next_instruction_normal();
    }

    //SLL
    else if(funct == 0b000000 && rs == 0){
        
        registers.write_reg(rd, registers.read_reg(rt) << shamt);

        registers.next_instruction_normal();
    }

    //SLLV
    else if(funct == 0b000100 && shamt == 0){

        registers.write_reg(rd, registers.read_reg(rt) << (registers.read_reg(rs) & 0x1F));

        registers.next_instruction_normal();
    }

    //SLT
    else if(funct == 0b101010 && shamt == 0){

        int32_t rs_signed = registers.read_reg(rs);
        int32_t rt_signed = registers.read_reg(rt);

        if(rs_signed < rt_signed){
            registers.write_reg(rd,1);
        }
        else{
            registers.write_reg(rd,0);
        }

        registers.next_instruction_normal();
    }

    //SLTU
    else if(funct == 0b101011 && shamt == 0){

        if(registers.read_reg(rs) < registers.read_reg(rt)){

            registers.write_reg(rd, 1);
        }
        else{

            registers.write_reg(rd, 0);
        }

        registers.next_instruction_normal();
    }

    //SRA
    else if(funct == 0b000011 && rs == 0){

        //convert to signed
        int32_t rt_signed = registers.read_reg(rt);

        registers.write_reg(rd, rt_signed >> shamt);

        registers.next_instruction_normal();
    }

    //SRAV
    else if(funct == 0b000111 && shamt == 0){

        //convert to signed
        int32_t rt_signed = registers.read_reg(rt);

        registers.write_reg(rd, rt_signed >> (registers.read_reg(rs) & 0x1F));

        registers.next_instruction_normal();
    }

    //SRL
    else if(funct == 0b000010 && rs == 0){

        registers.write_reg(rd, registers.read_reg(rt) >> shamt);

        registers.next_instruction_normal();
    }

    //SRLV
    else if(funct == 0b000110 && shamt == 0){

        registers.write_reg(rd, registers.read_reg(rt) >> (registers.read_reg(rs) & 0x1F));

        registers.next_instruction_normal();
    }

    //SUB
    else if(funct == 0b100010 && shamt == 0){

        int32_t a = registers.read_reg(rs);
        int32_t b = registers.read_reg(rt);

        int32_t result = a - b;

        if((((a & 0x80000000) == 0) && ((b & 0x80000000) != 0) && ((result & 0x80000000) != 0)) || (((a & 0x80000000) != 0) && ((b & 0x80000000) == 0) && ((result & 0x80000000) == 0))){

            exit(-10);
        }
        
        registers.write_reg(rd, result);

        registers.next_instruction_normal();
    }

    //SUBU
    else if(funct == 0b100011 && shamt == 0){

        registers.write_reg(rd, registers.read_reg(rs) - registers.read_reg(rt));

        registers.next_instruction_normal();
    }

    //XOR
    else if(funct == 0b100110 && shamt == 0){

        registers.write_reg(rd, registers.read_reg(rs) ^ registers.read_reg(rt));

        registers.next_instruction_normal();
    }


    else{

        // std::cerr << "not a valid R type instruction" << std::endl;
        //not a valid instruction (for now it might also not be implemented yet)
        exit(-12);
    }
}

void itype(uint8_t opcode, uint8_t rs, uint8_t rt, uint16_t immediate, mips_memory& memory, mips_registers& registers){

    //LUI
    if(opcode == 0b001111 && rs == 0){

        // std::cerr << "entered LUI" << std::endl;

        uint32_t temp = immediate << 16; 

        registers.write_reg(rt,temp);

        registers.next_instruction_normal();
    }

    //ADDIU
    else if(opcode == 0b001001){
        
        // std::cerr << "entered ADDIU" << std::endl;
        //sign extend the immediate. The current immediate is uint16_t, so i first have to convert it into a signed 16bit type, and then into a 32bit to sign extend properly
        int16_t signed_immediate = immediate;
        int32_t sign_extended_immediate = signed_immediate;

        // std::cerr << "signed_extended_value: " << std::hex << sign_extended_immediate << std::endl;

        //do the calculation
        int sum = registers.read_reg(rs) + sign_extended_immediate;

        // std::cerr << "sum: " << sum << std::endl;

        registers.write_reg(rt, sum);

        // std::cerr << "contents of rt: " << registers.read_reg(rt) << std::endl;

        registers.next_instruction_normal();
    }

    //SW
    else if(opcode == 0b101011){
        
        // std::cerr << "entered SW" << std::endl;

        //sign extend the immediate
        int16_t signed_immediate = immediate;
        int32_t sign_extended_immediate = signed_immediate;

        //work out the address
        uint32_t address = registers.read_reg(rs) + sign_extended_immediate;

        // std::cerr << "SW address: " << std::hex << address << std::endl;
        // std::cerr << "contents of rt: " << registers.read_reg(rt) << std::endl;

        //exceptions
    
        //write the contents of register rt to memory
        memory.write_DATA(registers.read_reg(rt), address);

        registers.next_instruction_normal();
    }

    //LW
    else if(opcode == 0b100011){

        // std::cerr << "entered LW" << std::endl;

        //sign extend the immediate
        int16_t signed_immediate = immediate;
        int32_t sign_extended_immediate = signed_immediate;

        //work out the address
        uint32_t address = registers.read_reg(rs) + sign_extended_immediate;

        // std::cerr << "address is " << std::hex << address << std::endl;
  
        uint32_t data = memory.read_DATA(address);
        registers.write_reg(rt, data);
        
        registers.next_instruction_normal();
    }

    //ORI
    else if(opcode == 0b001101){

        // std::cerr << "entered ORI" << std::endl;

        //zero extend the immeditate to 32bit
        uint32_t zero_extended_immediate = immediate;
        
        //do the calculation:
        uint32_t result = zero_extended_immediate | registers.read_reg(rs);

        registers.write_reg(rt, result);

        registers.next_instruction_normal();
    }

    //BNE
    else if(opcode == 0b000101){

        // std::cerr << "entered BNE" << std::endl;

        
        int16_t shifted_offset = immediate;
        int32_t offset = shifted_offset << 2;

        uint32_t address = registers.read_pc() + offset + 4;

        if(registers.read_reg(rs) != registers.read_reg(rt)){

            //branch delay
            if(registers.read_pc() < memory.read_LAST_INSTR_ADDRESS()){ //only executes the branch delay instruction if it there is an instruction after the JR

                instruction_decode_run(memory.read_INSTR(registers.read_pc()+4), memory, registers);
            }   

            //if the address points at 0x0, which means the program has finished execution, exit and indicate success
            if(address == 0){

                uint8_t exit_code = registers.read_reg(2);

                exit(exit_code);
            }

            //if the address is out of bounds, exit and indicate memory error
            else if(address < 0x10000000 || address >= 0x11000000){
               
                exit(-11);
            }

            registers.next_instruction_branch(address);
        }
        else{
            registers.next_instruction_normal();
        }
    }

    //ADDI
    else if(opcode == 0b001000){
        
        // std::cerr << "entered ADDI" << std::endl;

        //sign extend the immediate
        int16_t signed_immediate = immediate;
        int32_t sign_extended_immediate = signed_immediate;

        int32_t a = registers.read_reg(rs);

        int32_t sum = a + sign_extended_immediate;

        //checking for 2s complement overflow
        if((a > 0 && sign_extended_immediate > 0 && sum < 0) || (a < 0 && sign_extended_immediate < 0 && sum > 0)){ //overflow detected

            exit(-10);
        }
        else{

            registers.write_reg(rt, sum);
            
            registers.next_instruction_normal();
        }
    }

    //ANDI
    else if(opcode == 0b001100){
        
        uint32_t result = registers.read_reg(rs) & immediate;

        registers.write_reg(rt, result);

        registers.next_instruction_normal();
    }

    //BEQ
    else if(opcode == 0b000100){

        // std::cerr << "entered BEQ" << std::endl;

        
        int16_t shifted_offset = immediate;
        int32_t offset = shifted_offset << 2;

        uint32_t address = registers.read_pc() + offset + 4;

        if(registers.read_reg(rs) == registers.read_reg(rt)){

            //branch delay
            if(registers.read_pc() < memory.read_LAST_INSTR_ADDRESS()){ //only executes the branch delay instruction if it there is an instruction after the JR

                instruction_decode_run(memory.read_INSTR(registers.read_pc()+4), memory, registers);
            }   

            //if the address points at 0x0, which means the program has finished execution, exit and indicate success
            if(address == 0){

                uint8_t exit_code = registers.read_reg(2);

                exit(exit_code);
            }

            //if the address is out of bounds, exit and indicate memory error
            else if(address < 0x10000000 || address >= 0x11000000){
               
                exit(-11);
            }

            registers.next_instruction_branch(address);
        }
        else{
            registers.next_instruction_normal();
        }
    }

    //BGEZ
    else if(opcode == 0b000001 && rt == 0b00001){

        // std::cerr << "entered BGEZ" << std::endl;

        
        int16_t shifted_offset = immediate;
        int32_t offset = shifted_offset << 2;

        uint32_t address = registers.read_pc() + offset + 4;

        if((registers.read_reg(rs) & 0x80000000) == 0){ //check msb (as i store the registers as unsigned ints)

            //branch delay
            if(registers.read_pc() < memory.read_LAST_INSTR_ADDRESS()){ //only executes the branch delay instruction if it there is an instruction after the JR

                instruction_decode_run(memory.read_INSTR(registers.read_pc()+4), memory, registers);
            }   

            //if the address points at 0x0, which means the program has finished execution, exit and indicate success
            if(address == 0){

                uint8_t exit_code = registers.read_reg(2);

                exit(exit_code);
            }

            //if the address is out of bounds, exit and indicate memory error
            else if(address < 0x10000000 || address >= 0x11000000){
               
                exit(-11);
            }

            registers.next_instruction_branch(address);
        }
        else{
            registers.next_instruction_normal();
        }
    }

    //BGEZAL
    else if(opcode == 0b000001 && rt == 0b10001){

        // std::cerr << "entered BGEZAL" << std::endl;

        int16_t shifted_offset = immediate;
        int32_t offset = shifted_offset << 2;

        uint32_t address = registers.read_pc() + offset + 4;

        registers.write_reg(31, registers.read_pc()+8);

        if((registers.read_reg(rs) & 0x80000000) == 0){ //check msb (as i store the registers as unsigned ints)

            //branch delay
            if(registers.read_pc() < memory.read_LAST_INSTR_ADDRESS()){ //only executes the branch delay instruction if it there is an instruction after the JR

                instruction_decode_run(memory.read_INSTR(registers.read_pc()+4), memory, registers);
            }   

            //if the address points at 0x0, which means the program has finished execution, exit and indicate success
            if(address == 0){

                uint8_t exit_code = registers.read_reg(2);

                exit(exit_code);
            }

            //if the address is out of bounds, exit and indicate memory error
            else if(address < 0x10000000 || address >= 0x11000000){
               
                exit(-11);
            }

            registers.next_instruction_branch(address);
        }

        else{

            registers.next_instruction_normal();
        }
    }

    //BGTZ
    else if(opcode == 0b000111 && rt == 0b00000){

        // std::cerr << "entered BGTZ" << std::endl;

        int16_t shifted_offset = immediate;
        int32_t offset = shifted_offset << 2;

        uint32_t address = registers.read_pc() + offset + 4;

        if(((registers.read_reg(rs) & 0x80000000) == 0) && ((registers.read_reg(rs) != 0))){ //check msb (as i store the registers as unsigned ints)

            //branch delay
            if(registers.read_pc() < memory.read_LAST_INSTR_ADDRESS()){ //only executes the branch delay instruction if it there is an instruction after the JR

                instruction_decode_run(memory.read_INSTR(registers.read_pc()+4), memory, registers);
            }   

            //if the address points at 0x0, which means the program has finished execution, exit and indicate success
            if(address == 0){

                uint8_t exit_code = registers.read_reg(2);

                exit(exit_code);
            }

            //if the address is out of bounds, exit and indicate memory error
            else if(address < 0x10000000 || address >= 0x11000000){
               
                exit(-11);
            }

            registers.next_instruction_branch(address);
        }
        else{
            registers.next_instruction_normal();
        }
    }

    //BLEZ
    else if(opcode == 0b000110 && rt == 0b00000){

        // std::cerr << "entered BLEZ" << std::endl;

        
        int16_t shifted_offset = immediate;
        int32_t offset = shifted_offset << 2;

        uint32_t address = registers.read_pc() + offset + 4;

        // std::cerr << "contents of register rs " << std::hex << registers.read_reg(rs) << std::endl;
        

        if(((registers.read_reg(rs) & 0x80000000) != 0) || ((registers.read_reg(rs) == 0))){ //check msb (as i store the registers as unsigned ints)

            //branch delay
            if(registers.read_pc() < memory.read_LAST_INSTR_ADDRESS()){ //only executes the branch delay instruction if it there is an instruction after the JR

                instruction_decode_run(memory.read_INSTR(registers.read_pc()+4), memory, registers);
            }   

            //if the address points at 0x0, which means the program has finished execution, exit and indicate success
            if(address == 0){

                uint8_t exit_code = registers.read_reg(2);

                exit(exit_code);
            }

            //if the address is out of bounds, exit and indicate memory error
            else if(address < 0x10000000 || address >= 0x11000000){
               
                exit(-11);
            }

            registers.next_instruction_branch(address);
        }
        else{
            registers.next_instruction_normal();
        }
    }

    //BLTZ
    else if(opcode == 0b000001 && rt == 0b00000){

        // std::cerr << "entered BLTZ" << std::endl;
        
        int16_t shifted_offset = immediate;
        int32_t offset = shifted_offset << 2;

        uint32_t address = registers.read_pc() + offset + 4;

        if((registers.read_reg(rs) & 0x80000000) != 0){ //check msb (as i store the registers as unsigned ints)

            //branch delay
            if(registers.read_pc() < memory.read_LAST_INSTR_ADDRESS()){ //only executes the branch delay instruction if it there is an instruction after the JR

                instruction_decode_run(memory.read_INSTR(registers.read_pc()+4), memory, registers);
            }   

            //if the address points at 0x0, which means the program has finished execution, exit and indicate success
            if(address == 0){

                uint8_t exit_code = registers.read_reg(2);

                exit(exit_code);
            }

            //if the address is out of bounds, exit and indicate memory error
            else if(address < 0x10000000 || address >= 0x11000000){
               
                exit(-11);
            }

            registers.next_instruction_branch(address);
        }
        else{
            registers.next_instruction_normal();
        }
    }

    //BLTZAL
    else if(opcode == 0b000001 && rt == 0b10000){

        // std::cerr << "entered BLTZAL" << std::endl;

        int16_t shifted_offset = immediate;
        int32_t offset = shifted_offset << 2;

        uint32_t address = registers.read_pc() + offset + 4;

        registers.write_reg(31, registers.read_pc()+8);

        if((registers.read_reg(rs) & 0x80000000) != 0){ //check msb (as i store the registers as unsigned ints)

            //branch delay
            if(registers.read_pc() < memory.read_LAST_INSTR_ADDRESS()){ //only executes the branch delay instruction if it there is an instruction after the JR

                instruction_decode_run(memory.read_INSTR(registers.read_pc()+4), memory, registers);
            }   

            //if the address points at 0x0, which means the program has finished execution, exit and indicate success
            if(address == 0){

                uint8_t exit_code = registers.read_reg(2);

                exit(exit_code);
            }

            //if the address is out of bounds, exit and indicate memory error
            else if(address < 0x10000000 || address >= 0x11000000){
               
                exit(-11);
            }

            registers.next_instruction_branch(address);
        }

        else{

            registers.next_instruction_normal();
        }
    }

    //LB
    else if(opcode == 0b100000){

        //sign extend the immediate
        int16_t signed_immediate = immediate;
        int32_t extended_signed_immediate = signed_immediate;

        uint32_t address = registers.read_reg(rs) + extended_signed_immediate;

        uint8_t offset = address % 4;


        uint32_t word = memory.read_DATA(address - offset);
        int8_t byte;

        if(offset == 0){
            word = word >> 24;
        }
        else if(offset == 1){
            word = word >> 16;
        }
        else if(offset == 2){
            word = word >> 8;
        }

        byte = word & 0xFF;

        int32_t sign_extended_word = byte;
        registers.write_reg(rt, sign_extended_word);

        registers.next_instruction_normal();
    }

    //LBU
    else if(opcode == 0b100100){

        //sign extend the immediate
        int16_t signed_immediate = immediate;
        int32_t extended_signed_immediate = signed_immediate;

        uint32_t address = registers.read_reg(rs) + extended_signed_immediate;

        uint8_t offset = address % 4;

        uint32_t word = memory.read_DATA(address - offset);
        uint8_t byte;

        if(offset == 0){
            word = word >> 24;
        }
        else if(offset == 1){
            word = word >> 16;
        }
        else if(offset == 2){
            word = word >> 8;
        }

        byte = word & 0xFF;

        uint32_t sign_extended_word = byte;
        registers.write_reg(rt, sign_extended_word);

        registers.next_instruction_normal();
    }

    //LH
    else if(opcode == 0b100001){

        //sign extend the immediate
        int16_t signed_immediate = immediate;
        int32_t extended_signed_immediate = signed_immediate;

        uint32_t address = registers.read_reg(rs) + extended_signed_immediate;

        uint8_t offset = address % 4;

        int32_t word = memory.read_DATA(address - offset);

        int16_t hword;

        if(offset == 0){

            hword = (word >> 16);       
        }
        else if(offset == 2){

            hword = word;
        }
        else{
            exit(-11);
        }

        int32_t signed_extension_hword = hword;

        registers.write_reg(rt, signed_extension_hword);

        registers.next_instruction_normal();
    }

    //LHU
    else if(opcode == 0b100101){

        //sign extend the immediate
        int16_t signed_immediate = immediate;
        int32_t extended_signed_immediate = signed_immediate;

        uint32_t address = registers.read_reg(rs) + extended_signed_immediate;

        uint8_t offset = address % 4;

        uint32_t word = memory.read_DATA(address - offset);

        uint16_t hword;

        if(offset == 0){

            hword = (word >> 16);       
        }
        else if(offset == 2){

            hword = word;
        }
        else{
            exit(-11);
        }

        uint32_t zero_extension_hword = hword;

        registers.write_reg(rt, zero_extension_hword);

        registers.next_instruction_normal();
    }

    
    //LWL
    else if(opcode == 0b100010){

        //sign extend the immediate
        int16_t signed_immediate = immediate;
        int32_t extended_signed_immediate = signed_immediate;

        uint32_t address = registers.read_reg(rs) + extended_signed_immediate;
        uint8_t offset = address % 4;

        uint32_t original_word = registers.read_reg(rt);
        uint32_t memory_word = memory.read_DATA(address - offset);
    
        if(offset == 1){

            memory_word = (((memory_word << 8) & 0xFFFFFF00) | (original_word & 0xFF));
        }
        else if(offset == 2){

            memory_word = (((memory_word << 16) & 0xFFFF0000) | (original_word & 0xFFFF));
        }

        else if(offset == 3){

            memory_word = (((memory_word << 24) & 0xFF000000) | (original_word & 0xFFFFFF));
        }

        //if offset is 0 no need to change anything

        registers.write_reg(rt, memory_word);

        registers.next_instruction_normal();
    }

    //LWR
    else if(opcode == 0b100110){

        //sign extend the immediate
        int16_t signed_immediate = immediate;
        int32_t extended_signed_immediate = signed_immediate;

        uint32_t address = registers.read_reg(rs) + extended_signed_immediate;
        uint8_t offset = address % 4;

        uint32_t original_word = registers.read_reg(rt);
        uint32_t memory_word = memory.read_DATA(address - offset);
    
        if(offset == 0){

            memory_word = (((memory_word >> 24 ) & 0xFF) | (original_word & 0xFFFFFF00));
        }
        else if(offset == 1){

            memory_word = (((memory_word >> 16) & 0xFFFF) | (original_word & 0xFFFF0000));
        }

        else if(offset == 2){

            memory_word = (((memory_word >> 8) & 0xFFFFFF) | (original_word & 0xFF000000));
        }

        //if offset is 3 no need to change anything

        registers.write_reg(rt, memory_word);

        registers.next_instruction_normal();
    }

    //SB
    else if(opcode == 0b101000){

        // std::cerr << "entered SB" << std::endl;

        //sign extend the immediate
        int16_t signed_immediate = immediate;
        int32_t extended_signed_immediate = signed_immediate;

        uint32_t address = registers.read_reg(rs) + extended_signed_immediate;

        // std::cerr << "Address of data is " << std::hex << address << std::endl;

        uint8_t offset = address % 4;

        uint32_t memory_word;

        if(address - offset != 0x30000004){

            memory_word = memory.read_DATA(address - offset);
        }
        else{

            memory_word = 0;
        }

        // std::cerr << "Memory word is " << std::hex << memory_word << std::endl;

        uint32_t byte = registers.read_reg(rt) & 0xFF;

        if(offset == 0){

            memory_word = (memory_word & 0xFFFFFF) | (byte << 24);
        }
        else if(offset == 1){

            memory_word = (memory_word & 0xFF00FFFF) | (byte << 16);
        }
        else if(offset == 2){

            memory_word = (memory_word & 0xFFFF00FF) | (byte << 8);
        }
        else if(offset == 3){

            memory_word = (memory_word & 0xFFFFFF00) | byte;
        }

        memory.write_DATA(memory_word , address - offset);

        registers.next_instruction_normal();
    }

    //SH
    else if(opcode == 0b101001){

        //sign extend the immediate
        int16_t signed_immediate = immediate;
        int32_t extended_signed_immediate = signed_immediate;

        uint32_t address = registers.read_reg(rs) + extended_signed_immediate;
        uint8_t offset = address % 4;
        
        uint32_t memory_word;

        if(address - offset != 0x30000004){

            memory_word = memory.read_DATA(address - offset);
        }
        else{

            memory_word = 0;
        }

        uint32_t LSB = registers.read_reg(rt) & 0xFFFF;

        if(offset == 0){
            memory_word = (memory_word & 0xFFFF) | (LSB << 16) ;
        }
        else if(offset == 2){
            memory_word = (memory_word & 0xFFFF0000) | LSB;
        }
        else{
            exit(-11);
        }

        memory.write_DATA(memory_word, address - offset);

        registers.next_instruction_normal();
    }

    //SLTI
    else if(opcode == 0b001010){

        //make the contents of rs signed
        int32_t signed_rs = registers.read_reg(rs);

        //make the immediate signed and 32 bits
        int16_t signed_immediate = immediate;
        int32_t signed_extended_immediate = signed_immediate;

        if(signed_rs < signed_extended_immediate){
            registers.write_reg(rt, 1);
        }
        else{
            registers.write_reg(rt, 0);
        }

        registers.next_instruction_normal();
    }

    //SLTIU
    else if(opcode == 0b001011){

        //sign extend the immediate
        int16_t signed_immediate = immediate;
        int32_t signed_extended_immediate = signed_immediate;

        //make the sign extended immediate an unsigned type
        uint32_t unsigned_immediate = signed_extended_immediate;

        if(registers.read_reg(rs) < unsigned_immediate){
            registers.write_reg(rt, 1);
        }
        else{
            registers.write_reg(rt, 0);
        }

        registers.next_instruction_normal();
    }

    //XORI
    else if(opcode == 0b001110){

        uint32_t extended_immediate = immediate;

        registers.write_reg(rt, registers.read_reg(rs) ^ extended_immediate);

        registers.next_instruction_normal();
    }


    else{

        // std::cerr << "not a valid I type instruction" << std::endl;
        //invalid opcode or not implemented yet
        exit(-12);

    }
}

void jtype(uint8_t opcode, uint32_t short_address, mips_memory& memory, mips_registers& registers){
    
    //J
    if(opcode == 0b000010){

        // std::cerr << "entered J" << std::endl;

        uint32_t address = (short_address << 2) | (registers.read_pc() & 0xF0000000);

        // std::cerr << "address is " << std::hex << address << std::endl;
        //branch delay
        if(registers.read_pc() < memory.read_LAST_INSTR_ADDRESS()){ //only executes the branch delay instruction if it there is an instruction after

            instruction_decode_run(memory.read_INSTR(registers.read_pc()+4), memory, registers);
        }     

        //if the address points at 0x0, which means the program has finished execution, exit and indicate success
        if(address == 0){

            uint8_t exit_code = registers.read_reg(2);

            exit(exit_code);
        }

        //if the address is out of bounds, exit and indicate memory error
        else if(address < 0x10000000 || address >= 0x11000000){
               
            exit(-11);
        }

        registers.next_instruction_branch(address);
    }

    //JAL
    else if(opcode == 0b000011){

        // std::cerr << "entered JAL" << std::endl;

        uint32_t address = (short_address << 2) | (registers.read_pc() & 0xF0000000);

        registers.write_reg(31, registers.read_pc() + 8);

        //branch delay
        if(registers.read_pc() < memory.read_LAST_INSTR_ADDRESS()){ //only executes the branch delay instruction if it there is an instruction after

            instruction_decode_run(memory.read_INSTR(registers.read_pc()+4), memory, registers);
        }  

        //if the address points at 0x0, which means the program has finished execution, exit and indicate success
        if(address == 0){

            uint8_t exit_code = registers.read_reg(2);

            exit(exit_code);
        }

        //if the address is out of bounds, exit and indicate memory error
        else if(address < 0x10000000 || address >= 0x11000000){
               
            exit(-11);
        }

        registers.next_instruction_branch(address);
    }

    else{
        

        // std::cerr << "invalid J type instruction, current PC: " << std::hex << registers.read_pc() << std::endl;
        exit(-12);
    } 
}



