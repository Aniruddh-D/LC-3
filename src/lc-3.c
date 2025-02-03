/*      LC-3 Simulator
*       Project Inspired by: https://justinmeiners.github.io/lc3-vm/
*       Made by: Aniruddh-D
*       The LC-3 is a simple computer architecture that was designed to help students learn the basics of computer architecture.
*       The LC-3 has a simple instruction set, a small number of registers, and a small amount of memory.
*                - 16 total opcodes, each of which is 4 bits.
*                - 10 total registers, each of which is 16 bits.
*                - 65536 locations of memory.
*                - 3 condition flags: POS (Positive), ZRO (Zero), NEG (Negative).
*                - 5 total condition flags, each of which is 1 bit.
*                - 8 general purpose registers (R0-R7), 1 program counter (PC) register, and 1 condition flags (COND) register.
*                - 4 total condition flags, each of which is 1 bit.
*/
//@@diff: {Includes}
#include <stdio.h>
#include <stdint.h>
#include <signal.h>
/* for windows based OS*/
#include <Windows.h>
#include <conio.h>  // _kbhit


/* Remove from comment FOR UNIX based systems 

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/termios.h>
#include <sys/mman.h>
*/


enum
{
    R_R0 = 0,  
    R_R1,     
    R_R2,    
    R_R3,
    R_R4,
    R_R5,
    R_R6,
    R_R7,
    R_PC, // Program Counter
    R_COND, // Condition Flags
    R_COUNT // Total number of registers
};




// @@diff : {Condition Flags}
/*
*   The condition flags register (COND) is used to store information 
*   about the most recent operation that was performed.
*   The condition flags are as follows:
*       - 0000: POS (Positive) // The most recent operation resulted in a positive value
*       - 0001: ZRO (Zero)     // The most recent operation resulted in a value of zero
*       - 0010: NEG (Negative) // The most recent operation resulted in a negative value
*/
enum
{
    FL_POS = 1 << 0, // P
    FL_ZRO = 1 << 1, // Z
    FL_NEG = 1 << 2, // N
};

// @@diff : {OpCodes}

/*  
*   Instructions have both an opcode which indicates the kind of task to perform 
*   and a set of parameters which provide inputs to the task being performed
*   Each opcode represents one task that the CPU “knows” how to do.
*   
*   The LC-3 has 16 total opcodes, each of which is 4 bits.
*   The opcodes are as follows:
*       - 0001: ADD (R1 = R2 + R3)
*       - 0101: AND (R1 = R2 & R3)
*       - 0000: BR  (Branch)
*       - 1100: JMP (Jump)
*       - 0100: JSR (Jump to Subroutine)
*       - 0100: JSRR (Jump to Subroutine)
*       - 0010: LD (LOAD)
*       - 1010: LDI (LOAD Indirect)
*       - 0110: LDR (LOAD Register)
*       - 1110: LEA (LOAD Effective Address)
*       - 1001: NOT (NOT)
*       - 1000: RTI (Return from Interrupt)
*       - 0011: ST (Store)
*       - 1011: STI (Store Indirect)
*       - 0111: STR (Store Register)
*       - 1111: TRAP (TRAP aka system call)
*/
enum
{
    OP_BR = 0, // branch
    OP_ADD,    // add
    OP_LD,     // load
    OP_ST,     // store
    OP_JSR,    // jump register
    OP_AND,    // bitwise and
    OP_LDR,    // load register
    OP_STR,    // store register
    OP_RTI,    // unused
    OP_NOT,    // bitwise not
    OP_LDI,    // load indirect
    OP_STI,    // store indirect
    OP_JMP,    // jump
    OP_RES,    // reserved (unused)
    OP_LEA,    // load effective address
    OP_TRAP    // execute trap
};

enum 
{
    MR_KBSR = 0xFE00, /* keyboard status register */
    MR_KBDR = 0xFE02  /* keyboard data register */
};

enum
{
    TRAP_GETC = 0x20,   /* get character from keyboard, not echoed onto the terminal */
    TRAP_OUT = 0x21,     /* output a character */
    TRAP_PUTS = 0x22,   /* output a word string */
    TRAP_IN = 0x23,      /* get character from keyboard, echoed onto the terminal */
    TRAP_PUTSP = 0x24,  /* output a byte string */
    TRAP_HALT = 0x25    /* halt the program */
};


//  @@diff: {Memory Storage}
#define MEMORY_MAX (1<<16) 
uint16_t memory[MEMORY_MAX]; // 65536 Locations of memory


//  @@diff: {Registers}
/*  
*   The LC-3 has 10 total registers, each of which is 16 bits. 
*   Most of them are general purpose, but a few have designated roles: - 
*        - 8 general purpose registers (R0-R7) [can be used to perform any program calculations]
*        - 1 program counter (PC) register [an unsigned integer which is the address of the next instruction in memory to execute]
*        - 1 condition flags (COND) register [tells us information about the previous calculation.]
*/

// @@diff : {Register Storage}
uint16_t reg[R_COUNT]; // store the 10 registers in an array

HANDLE hStdin = INVALID_HANDLE_VALUE; // get the standard input handle
DWORD fdwMode, fdwOldMode; // get the mode of the standard input

void disable_input_buffering()
{
    hStdin = GetStdHandle(STD_INPUT_HANDLE); // get the standard input handle
    GetConsoleMode(hStdin, &fdwOldMode); // get the mode of the standard input
    
    fdwMode = fdwOldMode ^ ENABLE_ECHO_INPUT //no input echo 
                         ^ ENABLE_LINE_INPUT; //return when one or more characters are available.
    
    SetConsoleMode(hStdin, fdwMode); // set the mode of the standard input
    FlushConsoleInputBuffer(hStdin); // flush the input buffer                     
}

void restore_input_buffering()
{
    SetConsoleMode(hStdin, fdwOldMode); // set the mode of the standard input
}

uint16_t check_key()
{
    return WaitForSingleObject(hStdin, 1000) == WAIT_OBJECT_0 && _kbhit(); // check if a key is pressed
}

void handle_interrupt(int signal)
{
    restore_input_buffering(); // restore the input buffering
    printf("\n"); // output a new line
    exit(-2); // exit the program
}



/* Remove from comment FOR UNIX based systems 
struct termios original_tio;

void disable_input_buffering()
{
    tcgetattr(STDIN_FILENO, &original_tio);
    struct termios new_tio = original_tio;
    new_tio.c_lflag &= ~ICANON & ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
}

void restore_input_buffering()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
}

uint16_t check_key()
{
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    return select(1, &readfds, NULL, NULL, &timeout) != 0;
}
*/



// @@diff : {Sign Extend}
/*
*   [Padding]
*   Sign extension is the operation of increasing the number of bits of a binary number.
*   This is achieved by adding digits to the most significant side of the number.
*   The most significant bit of the original number is used as the extension bit.
*   The value of the extension bits is the same as the value of the most significant bit.
*   Sign extension is used to preserve the sign of a number when it is extended.
*   Sign extension is used to extend the length of a number without changing its value.
*/
uint16_t sign_extend(uint16_t x, int bit_count) // sign extend the number
{
    if((x >> (bit_count - 1)) & 1) // if the most significant bit is 1
    {
        x |= (0xFFFF << bit_count); // set the most significant bits to 1
    }
    return x;
}

uint16_t swap16(uint16_t x) // swap the value to big endian format
{
    return (x << 8) | (x >> 8); // shift the value by 8 bits and OR it with the value shifted by 8 bits
}

//@@diff : {Update Flags}

void update_flags(uint16_t r)
{
    if(reg[r] == 0)
    {
        reg[R_COND] = FL_ZRO;
    }
    else if (reg[r] >> 15) // 1 in the leftmost-bit indicates negative value
    {
        reg[R_COND] = FL_NEG;
    }
    else
    {
        reg[R_COND] = FL_POS;
    }

    
}
//@@diff : {Read Image File}
int read_image_file(FILE* file)
{
    /* the origin tells us where in memory to place the image */
    uint16_t origin;
    fread(&origin, sizeof(origin), 1, file); // read the origin from the file
    origin = swap16(origin); // swap the origin to big endian format

    //we know the maximum file size so we only need one fread().

    uint16_t max_read = MEMORY_MAX - origin; // calculate the maximum number of words that can be read.
    uint16_t* p = memory + origin; // set the pointer to the memory location
    size_t read = fread(p, sizeof(uint16_t), max_read, file); // read the image from the file

    // swap to little endian
    while(read-- > 0)
    {
        *p = swap16(*p); // swap the value to little endian format
        ++p;
    }
}

int read_image(const char* image_path)
{
    FILE *file = fopen(image_path, "rb"); // open the file in binary mode
    if(!file) // if the file is not opened
    {
        return 0;
    }
    read_image_file(file); // read the image file
    fclose(file); // close the file
    return 1;
}


void mem_write(uint16_t address, uint16_t val) // write the value to the memory location
{
    memory[address] = val; // write the value to the memory location
}

uint16_t mem_read(uint16_t address) // read the value from the memory location
{
    if(address == MR_KBSR) // if the address is the keyboard status register
    {
        if(_kbhit()) // if a key is pressed
        {
            memory[MR_KBSR] = (1 << 15); // set the keyboard status register to 1
            memory[MR_KBDR] = getchar(); // get the character from the keyboard
        }
        else
        {
            memory[MR_KBSR] = 0; // set the keyboard status register to 0
        }
    }
    return memory[address]; // return the value from the memory location
}



void print_state()
{
    printf("Registers:\n");
    for (int i = 0; i < R_COUNT; i++)
    {
        printf("R%d: 0x%04X\n", i, reg[i]);
    }
    printf("Memory:\n");
    for (int i = 0; i < MEMORY_MAX; i++)
    {
        if (memory[i] != 0)
        {
            printf("0x%04X: 0x%04X\n", i, memory[i]);
        }
    }
    printf("\n");
}

// @@diff : Main
int main(int argc, char* argv[])
{
    //@diff : {Load Arguments}
   
   if(argc<2)
   { 
        printf("lc3 [image-file1] ...\n"); //usage string
        exit(2);
   }
   for(int j=1; j<argc; j++) // loop through each argument read the image file
   {
        if(!read_image(argv[j])) // if the image file is not read
        {
            printf("failed to load image: %s\n", argv[j]);
            exit(1);
        }
   }

   signal(SIGINT, handle_interrupt); // handle the interrupt signal
   disable_input_buffering(); // disable the input buffering

   //@diff : {Setup}

   /*   since exactly one condition flag should be set at any 
   *    given time, set the Z flag 
   */
  reg[R_COND] = FL_ZRO;
  enum{PC_START = 0x3000}; // default PC starting position
  reg[R_PC] = PC_START; // default PC starting position

  int running = 1;
  while(running)
  {
    //Fetch
    uint16_t instr = mem_read(reg[R_PC]++);
    uint16_t op = instr >> 12;

    switch(op)
    {
        case OP_ADD:
            // @@ diff : {ADD}
            {
                uint16_t r0 = (instr >> 9) & 0x7; // The shift moves the register field to the lowest three bits. // 0x7(111 in binary) ensures only those three bits are preserved.
                uint16_t r1 = (instr >> 6) & 0x7; // first operand (SR1)
                uint16_t imm_flag = (instr >> 5) & 0x1; // immediate flag (0 = register, 1 = immediate) // 0x1 ensures only the lowest bit is preserved.
                
                if(imm_flag)
                {
                    uint16_t imm5 = sign_extend(instr & 0x1F, 5); // sign extend the 5-bit immediate to 16 bits
                    reg[r0] = reg[r1] + imm5; // perform the addition
                }
                else
                {
                    uint16_t r2 = instr & 0x7; // second operand (SR2)
                    reg[r0] = reg[r1] + reg[r2]; // perform the addition
                }

                update_flags(r0); // update the condition flags

            }

        break;

        case OP_AND:
            {
                // @@ diff : {AND}
                uint16_t r0 = (instr >> 9) & 0x7; // destination register
                uint16_t r1 = (instr >> 6) & 0x7; // first operand
                uint16_t imm_flag = (instr >> 5) & 0x1; // immediate flag
                if(imm_flag)
                {
                    uint16_t imm5 = sign_extend(instr & 0x1F, 5); // sign-extend the 5-bit immediate to 16 bits
                    reg[r0] = reg[r1] & imm5; // perform the AND
                }
                else
                {
                    uint16_t r2 = instr & 0x7; // second operand
                    reg[r0] = reg[r1] & reg[r2]; // perform the AND
                }
                update_flags(r0); // update the condition flags
            }
        break;

        case OP_NOT:      
        {
            // @@ diff : {NOT}
            uint16_t r0 = (instr >> 9) & 0x7; // destination register
            uint16_t r1 = (instr >> 6) & 0x7; // source register
            reg[r0] = ~reg[r1]; // perform the NOT
            update_flags(r0); // update the condition flags
        }
        break;

        case OP_BR:   // Branch
        {   // @@ diff : {BR}
            uint16_t pc_offset = sign_extend(instr & 0x1FF, 9); // sign-extend the offset to 16 bits
            uint16_t cond_flag = (instr >> 9) & 0x7; // condition flag
            if(cond_flag & reg[R_COND]) // if the condition flag is set
            {
                reg[R_PC] += pc_offset; // branch to the location
            }
        }    
        break;

        case OP_JMP:    // Jump
        {
            // @@ diff : {JMP}
            uint16_t r1 = (instr >> 6) & 0x7; // base register
            reg[R_PC] = reg[r1]; // set the PC to the base register
        }    
        break;

        case OP_JSR: // Jump to Subroutine/Register
        {
            // @@ diff : {JSR}
            uint16_t long_flag = (instr >> 11) & 1; // long flag (1 = long, 0 = short)
            reg[R_R7] = reg[R_PC]; // store the return address in R7
            
            if(long_flag) // if the long flag is set
            {
                uint16_t pc_offset = sign_extend(instr & 0x7FF, 11); // sign-extend the offset to 16 bits
                reg[R_PC] += pc_offset; // branch to the location / JSR
            }
            else
            {
                uint16_t r1 = (instr >> 6) & 0x7; // base register
                reg[R_PC] = reg[r1]; // set the PC to the base register / JSRR
            }

        }    
        break;

        case OP_LD:   // Load
        {    // @@ diff : {LD}
            uint16_t r0 = (instr >> 9) & 0x7; // destination register
            uint16_t pc_offset = sign_extend(instr & 0x1FF, 9); // sign-extend the offset to 16 bits
            
            reg[r0] = mem_read(reg[R_PC] + pc_offset); // read the value from the address in memory
            update_flags(r0); // update the condition flags
        }   
        break;

        case OP_LDI:    // Load Indirect
        {    // @@ diff : {LDI}
            uint16_t r0 = (instr >> 9) & 0x7; // destination register
            uint16_t pc_offset = sign_extend(instr & 0x1FF, 9); // sign-extend the offset to 16 bits
            // add pc_offset to the current PC, look at that memory location to get the final address
            reg[r0] = mem_read(mem_read(reg[R_PC] + pc_offset)); // read the value from the address in memory
            update_flags(r0); // update the condition flags
        }    
        break;

        case OP_LDR:    // Load Register
        {    // @@ diff : {LDR}
            uint16_t r0 = (instr >> 9) & 0x7; //destination register
            uint16_t r1 = (instr >> 6) & 0x7; // base register

            uint16_t offset = sign_extend(instr & 0x3F, 6); // sign-extend the offset to 16 bits
            reg[r0] = mem_read(reg[r1]+offset); // read the value from the address in memory
            update_flags(r0); // update the condition flags
        }    
        break;

        case OP_LEA: // Load Effective Address
        { // @@ diff : {LEA}
            uint16_t r0 = (instr >> 9) & 0x7; // destination register
            uint16_t pc_offset = sign_extend(instr & 0x1FF, 9); // sign-extend the offset to 16 bits.
            reg[r0] = reg[R_PC] + pc_offset; // load the effective address
            update_flags(r0); // update the condition flags
        }
        break;

        case OP_ST:   // Store
        {
            // @@ diff : {ST}
            uint16_t r0 = (instr >> 9) & 0x7; // source register
            uint16_t pc_offset = sign_extend(instr & 0x1FF, 9); // sign-extend the offset to 16 bits
            mem_write(reg[R_PC] + pc_offset, reg[r0]); // write the value to the address in memory
        }    
        break;

        case OP_STI:
            // @@ diff : {STI}
            {
                uint16_t r0 = (instr >> 9) & 0x7;   // Extracts the destination register (r0) from the instruction
                uint16_t pc_offset = sign_extend(instr & 0x1FF, 9); 
                mem_write(mem_read(reg[R_PC] + pc_offset), reg[r0]);
            }
        break;

        case OP_STR:
            // @@ diff : {STR}
            {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t r1 = (instr >> 6) & 0x7;
                uint16_t offset = sign_extend(instr & 0x3F, 6);
                mem_write(reg[r1] + offset, reg[r0]);
            }
        break;

        case OP_TRAP:
            // @@ diff : {TRAP}
            {
                reg[R_R7] = reg[R_PC];
                switch(instr & 0xFF)
                {
                    case TRAP_GETC:
                    {
                        //@TRAP_GETC
                        reg[R_R0] = (uint16_t)getchar(); // read a single ASCII character
                        update_flags(R_R0); // update the condition flags
                    }
                    break;

                    case TRAP_OUT:
                    {
                        //@TRAP_OUT
                        putc((char)reg[R_R0], stdout); // output a single ASCII character
                        fflush(stdout); // flush the output
                    }
                    break;

                    case TRAP_PUTS:
                    {
                        //@TRAP_PUTS
                        uint16_t* c = memory + reg[R_R0]; // get the address of the string
                        while(*c)
                        {
                            putc((char)*c, stdout); // output the character
                            ++c;
                        }
                        fflush(stdout); // flush the output
                    }
                    break;

                    case TRAP_IN:
                    {
                        //@TRAP_IN
                        printf("Enter a character: ");
                        char c = getchar(); // read a single ASCII character
                        putc(c, stdout); // echo the character
                        fflush(stdout); // flush the output
                        reg[R_R0] = (uint16_t)c; // store the character in R0
                        update_flags(R_R0); // update the condition flags
                    }
                    break;

                    case TRAP_PUTSP:
                    {
                        //@TRAP_PUTSP
                        /* one char per byte (two bytes per word) here we need to swap back to
                        big endian format */
                        uint16_t* c = memory + reg[R_R0]; // get the address of the string
                        while(*c)
                        {
                            char char1 = (*c) & 0xFF; // get the first ASCII character
                            putc(char1, stdout); // output the character
                            char char2 = (*c) >> 8; // get the second ASCII character
                            if(char2) putc(char2, stdout); // output the character if it is not null
                            ++c;
                        }
                        fflush(stdout); // flush the output
                    }
                    break;

                    case TRAP_HALT:
                    {
                        //@TRAP_HALT
                        puts("HALT"); // output "HALT"
                        fflush(stdout); // flush the output
                        running = 0; // stop the program
                    }
                    break;
                }
            }
        break;

        case OP_RES:
        case OP_RTI:

        default:
            // @@ diff : {BAD OPCODE}
            abort();
        break;
    }

   print_state(); // print the state of the program
    // @@ diff : {Shutdown}
  }

  restore_input_buffering(); // restore the input buffering
}
