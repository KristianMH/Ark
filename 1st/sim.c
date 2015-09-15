#include <stdio.h>
#include "mips32.h"
#include <stdint.h>
#include "elf.h"
#include <stdlib.h>

#define ERROR_INVALID_ARGS 1
#define ERROR_IO_ERROR 2
#define ERROR_UNKNOWN_OPCODE 3
#define MEMSZ 640*1024
static uint32_t PC = 0;
static uint32_t regs[32];
static size_t instr_cnt = 0;
static unsigned char mem[640*1024];

int show_status(){
  printf("Executed %zu instruction(s).\n", instr_cnt); 
  printf("PC = 0x%x\n", PC);
  printf("at = 0x%x\n", regs[1]);
  printf("v0 = 0x%x\n", regs[2]);
  printf("v1 = 0x%x\n", regs[3]);

  for (int i = 8; i < 16; i++) {
    printf("t%d = 0x%x\n",i-8, regs[i]);
  }
  printf("sp = 0x%x\n", regs[29]);
  printf("fp = 0x%x\n", regs[30]);
  
  return 0;
}

int read_config_stream(FILE *file){
  for (int i = 8; i < 15; i++){
    uint32_t v;
    int count = fscanf(file, "%u", &v);
    if (count != 1){
      return 1;
    }
    regs[i] = v;
  }

  if (fclose(file) != 0){
    return ERROR_IO_ERROR;  
  }
  return 0;
}

int read_config(const char *path) {
  
  FILE *fdesc = fopen(path, "r");
  if(fdesc == NULL){
    printf("Could not open file\n");
    return ERROR_IO_ERROR;
  }

  read_config_stream(fdesc);
  
  printf("Readfile!\n");
  return 0;
}
int intrp_r(uint32_t instr){
  if (GET_FUNCT(instr) == FUNCT_SYSCALL) return 1;
  return 4;

}
int interp_inst(uint32_t instr){
  //gets opcode by GET_UPCODE(instr) and do swicth case.r  
  switch(GET_OPCODE(instr)){
  case FUNCT_SYSCALL :
    return 1;
  case OPCODE_R:
    return intrp_r(instr);
  default : 
    return ERROR_UNKNOWN_OPCODE;
  }
  
  return 0;
}
int interp(){
  for (;;){
    instr_cnt ++;
    int count = interp_inst(GET_BIGWORD(mem,PC)); // gets instr from PC in mem array
    if (count == 1){ //sees the syscall function;
      printf("hello this is the syscall function\n");
      return 0;
    }
    if (count == 4){ // temp for bugfixing
      return 0;
    }
    if (count == ERROR_UNKNOWN_OPCODE){ //error occured eg. unsupported function
      return ERROR_UNKNOWN_OPCODE;
    }
    PC +=4;
  }
  return 0;
}
int main(int argc, char *argv[]){
   if(argc != 3){
     printf("Insufficient arguments\n");
     return ERROR_INVALID_ARGS;
   }
   // checks return value of read cfg FILE.
   if(read_config(argv[1])==0){ 
     //return value of elf_dump. 
     int elfreturn = elf_dump(argv[2], &PC, &mem[0], MEMSZ);
     if (elfreturn == 0){ 
       regs[29] = MIPS_RESERVE+MEMSZ-4;
       interp();
     }
   }
   return show_status();

}

