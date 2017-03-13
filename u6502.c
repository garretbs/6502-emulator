//Garret Stevens

#include <stdio.h>
#include <stdlib.h>

#define RAM_SIZE 8192
#define RANDOM_MEM_SIZE 8192
#define MEM_SIZE 8

unsigned char ram[RAM_SIZE];
unsigned char *memory_map[MEM_SIZE];

//FLAGS
int HALT = 0;

//Instructions

#define ADC 3 //Immediate
#define STA 4 //Absolute
#define LDA 5 //Immediate

#define IMMEDIATE 2
#define ABSOLUTE 3

#define RELATIVE 4

#define JMP 2 //Absolute
#define BRK 0X00
#define BCC 0x90 //Relative
#define BCS 0XB0 //Relative
#define BNE 0XD0 //Relative
#define BEQ 0XF0 //Relative

//stuff
int accumulator = 0;
int program_counter = 0;
int instruction_register = 0;

//Status Register
int n; //Negative
int v; //Overflow
int b; //Break
int d; //Decimal (use BCD for arithmetics)
int i; //Interrupt (IRQ disable)
int z; //Zero
int c; //Carry



void create_memory_map(){
	memory_map[0] = ram;
	
	srand(4189687);
	unsigned char random_memory[RANDOM_MEM_SIZE];
	for(int i=0;i<RANDOM_MEM_SIZE;++i){
		random_memory[i] = rand() % 255;
	}
	for(int i=1;i<=MEM_SIZE;++i){
		memory_map[i] = random_memory;
	}
}

unsigned char access_ram(int write, short address, unsigned char out_byte){
	if(write){
		memory_map[0][address] = out_byte;
	}else{
		return memory_map[0][address];
	}
}

unsigned char access_random_mem(int write, short address, unsigned char out_byte){
	if(write){
		//do nothing
	}else{
		return memory_map[1][address];
	}
}

unsigned char access_memory(int write, short address, unsigned char out_byte){
	short block = address >> 9;
    short true_address = address & 0x1ff;
	
	if(block == 0){
		return access_ram(write, true_address, out_byte);
	}else{
		return access_random_mem(write, true_address, out_byte);
	}
}

int operand1;
int operand2;
int aaa; int bbb; int cc
int offset;
int temp1; int temp2;
int bothNegative;

void execute_next_instruction(){
	
	instruction_register = access_memory(0, program_counter++, 0);
	
	aaa = (instruction_register & 0b11100000) >> 5;
	bbb = (instruction_register & 0b00011100) >> 2;
	cc 	= instruction_register & 0b00000011;
	
	if(cc == 01){
		
		switch(bbb){
			case IMMEDIATE:
				operand1 = access_memory(0, program_counter++, 0);
				break;
			case ABSOLUTE:
				offset = access_memory(0, program_counter++, 0);
				offset = (offset << 8) | access_memory(0, program_counter++, 0);
				operand1 = offset;
				break;
		}
		
		switch(aaa){
			case ADC: //add with carry
				temp1 = operand1 & 0x80; //negative flag
				temp2 = accumulator & 0x80; //ditto
				
				if(temp1 ^ temp2){ //if both are the same sign
				
					v = 0; //no overflow
					if(temp1 & temp2){ //both are negative
						bothNegative = 1;
					}else{
						bothNegative = 0;
					}
					
				}else{
					v = 1; //set overflow flag
				}
					
				temp1 = accumulator + operand1;
				
				if(temp1 & 0x100){ //no carry
					c = 0;
				}else{
					c = 1;
				}
				
				accumulator = temp1 & 0xff;
				z = 1;
				n = 1;
				
				break;
			case LDA: //load variable into accumulator
				accumulator = access_memory(0, operand1, 0);
				break;
			case STA: //store accumulator's value into variable
				access_memory(1, operand1, accumulator);
				break;
		}		
		
	}else{
		switch(bbb){
			
			case RELATIVE:
				offset = access_memory(0, program_counter++, 0);
				offset = (offset >> 7) == 1 ? (-1 * (offset & 0x7f)) ? (offset & 0x7f);
				break;
		}
		
		switch(instruction_register){
			case BRK:
				HALT = 1;
				program_counter++;
				break;
		}
			
		
		switch(aaa){
			case BCC: //branch on carry clear
				if(!c){
					program_counter+=offset;
				}
				break;
			case BCS: //branch on carry set
				if(c){
					program_counter+=offset;
				}
				break;
			case BEQ: //branch on equal
				if(z){
					program_counter+=offset;
				}
				break;
			case BNE: //branch on not equal
				if(!z){
					program_counter+=offset;
				}
				break;
			case JMP: //jump. transfers execution to the absolute address
				program_counter = (operand1 << 8) | operand2;
				break;
		}
	
	}
}

void run(int start_address){
	//continuously loops while calling executeNextInstruction() until a BRK sets the global halt flag
	n = 0;
    v = 0;
    b = 1;
    d = 0;
    i = 1;
    z = 0;
    c = 0;
	
	create_memory_map();
	
	while(!HALT){
		execute_next_instruction();
	}
}

void main(){
	run(0);
}