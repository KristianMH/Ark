#include <stdio.h>
#include "mips32.h"
#include <stdint.h>

#define ERROR_INVALID_ARGS 1
#define ERROR_IO_ERROR 2

static uint32_t PC = 0;
static uint32_t regs[32];
static size_t instr_cnt = 0;

int show_status(){
  printf("Executed %zu instruction(s).\n", instr_cnt); 
  printf("%u\n", PC);
  
  return 0;
}

int read_config_stream(FILE *file){
  for (int i = 8; i < 15; i++){
    uint32_t v;
    if (fscanf(file, "%u", &v) != 0){
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

int main(int argc, char *argv[]){
  printf("%x\n", PC);
  regs[1] = regs[1];
  if(argc == 3){
    return read_config(argv[1]);
  }
  printf("Insufficient arguments\n");
  return ERROR_INVALID_ARGS;

  show_status();
}

