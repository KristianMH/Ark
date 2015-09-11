#include <stdio.h>
#include "mips32.h"
#include <stdint.h>
#include "elf.h"

#define ERROR_INVALID_ARGS 1
#define ERROR_IO_ERROR 2

#define MEMSZ sizeof mem;

static uint32_t PC = 0;
static uint32_t regs[32];
static size_t instr_cnt = 0;
static unsigned char mem[640];

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
    printf("%d\n", count);
    if (count != 1){
      return 1;
    }
    regs[i] = v;
    printf("%d\n", v);
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

int main(int argc, char *argv[]){
   if(argc == 3){
    read_config(argv[1]);
  } else {
    printf("Insufficient arguments\n");
    return ERROR_INVALID_ARGS;
  }
   show_status();
   elf_dump(argv[2], &PC, &mem[0], MEMSZ);

   return 0;
}

