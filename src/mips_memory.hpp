
#ifndef MIPS_MEMORY
#define MIPS_MEMORY   //making sure it is not included twice

class mips_memory{

    public:

    //constructor
    mips_memory();

    ///////////////////////////////
    ///// READING FROM MEMORY /////
    ///////////////////////////////

    //read instruction from ADDR_INSTR. Memory location is from 0x10000000 to 0x11000000-1.
    uint32_t read_INSTR(int memory_location); 

    //read data from ADDR_DATA. Memory location is from 0x20000000 to 0x24000000-1.
    uint32_t read_DATA(int memory_location); 


    ///////////////////////////////
    ////// WRITING TO MEMORY //////
    ///////////////////////////////

    //Load instructions into ADDR_INSTR
    void copy_ADDR_INSTR(char* source);

    //sets the size of the bin file (used to keep track where the end of instructions is)
    void set_INSTR_SIZE(int size);

    //write data to memory. Memory location is from 0x20000000 to 0x24000000-1.
    void write_DATA(uint32_t data, int memory_location);

    //return the LAST_INSTR_INDEX (if the PC equals that then the program reached the end)
    uint32_t read_LAST_INSTR_ADDRESS();


    //maybe some kind of flags for testing?

    //might need other stuff

    private:

    std::vector<uint8_t> ADDR_INSTR;
    int INSTR_SIZE; // added here so we don't have to manually count it too many times. Counted in bytes.
    int LAST_INSTR_ADDRESS; //shows the last instruction, used to compare with PC to check if the program finished.

    std::vector<uint8_t> ADDR_DATA;

};

#endif
