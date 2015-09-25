#include <stdio.h>
#include "mips32.h"
#include <stdint.h>
#include "elf.h"
#include <stdlib.h>

#define ERROR_INVALID_ARGS 1
#define ERROR_IO_ERROR 2
#define ERROR_UNKNOWN_OPCODE 3
#define ERROR_UNKNOWN_FUNCT 4
#define MEMSZ 640*1024
#define SAW_SYSCALL 5
static uint32_t PC = 0;
static uint32_t regs[32];
static size_t instr_cnt = 0;
static unsigned char mem[640*1024];
static size_t cycles = 0;
struct preg_if_id {
  uint32_t inst;
  
};
struct preg_id_ex {
  bool mem_read;
  bool mem_write;
  bool reg_write;
  uint32_t rt = GET_RT(if_id.inst);
  uint32_t rs_value = regs[GET_RS(if_id.inst)];
  uint32_t rt_value = regs[rt];
  uint32_t sign_ext_imm = SIGN_EXTEND(GET_IMM(if_id.inst));
  uint32_t funct;
};
static struct preg_if_id if_id;
static struct preg_if_id id_ex;
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
// reads the variables from cfg file
int read_config_stream(FILE *file){
  for (int i = 8; i < 15; i++){
    uint32_t v;
    int count = fscanf(file, "%u", &v);
    // checks if fscanf failed to read
    if (count != 1){
      return 1;
    }
    regs[i] = v;
  }

  return 0;
}

int read_config(const char *path) {
  
  FILE *fdesc = fopen(path, "r");
  if(fdesc == NULL){
    printf("Could not open file\n");
    return ERROR_IO_ERROR;
  }

  if (read_config_stream(fdesc) == 1){
    printf("unsuccesfully initialized registers, check cfg file");
    return ERROR_IO_ERROR;
  }
  if (fclose(fdesc) != 0) {
    return ERROR_IO_ERROR;
  }
  return 0;
}

int addu(uint32_t instr){
  regs[GET_RD(instr)] = regs[GET_RS(instr)]+regs[GET_RT(instr)];
  return 0;
}

int addiu(uint32_t instr){
  regs[GET_RT(instr)] = regs[GET_RS(instr)]+SIGN_EXTEND(GET_IMM(instr));
  return 0;
}

int bne(uint32_t instr){
  if (regs[GET_RT(instr)] != regs[GET_RS(instr)]){
    PC += (SIGN_EXTEND(GET_IMM(instr)) << 2)  ; 
}
  return 0;
}

int beq(uint32_t instr){
   if (regs[GET_RT(instr)] == regs[GET_RS(instr)]){
    PC += (SIGN_EXTEND(GET_IMM(instr)) << 2) ; 
   }
  return 0;
}

int subu(uint32_t instr){
  regs[GET_RD(instr)] = regs[GET_RS(instr)]-regs[GET_RT(instr)];
  return 0;
}

int slt(uint32_t instr){
  regs[GET_RD(instr)] = regs[GET_RS(instr)] < regs[GET_RT(instr)] ? 1:0;
  return 0;
}

int sll(uint32_t instr){
  regs[GET_RD(instr)] = regs[GET_RT(instr)] << GET_SHAMT(instr);
  return 0;
}

int srl(uint32_t instr){
  regs[GET_RD(instr)] = regs[GET_RS(instr)] >> GET_SHAMT(instr);
  return 0;
}

int slti(uint32_t instr){
  regs[GET_RT(instr)] = regs[GET_RS(instr)] < GET_IMM(instr) ? 1:0;
  return 0;      
}

int and(uint32_t instr){
  regs[GET_RD(instr)] = regs[GET_RS(instr)] & regs[GET_RT(instr)];
  return 0;
}

int or(uint32_t instr){
  regs[GET_RD(instr)] = regs[GET_RS(instr)] | regs[GET_RT(instr)];
  return 0;
}

int nor(uint32_t instr){
  regs[GET_RD(instr)] = ~(regs[GET_RS(instr)] | regs[GET_RT(instr)]);
  return 0;
}

int j(uint32_t instr){
  PC = (PC & MS_4B) | (GET_ADDRESS(instr) << 2);
  return 0;
}

int jr (uint32_t instr){
  PC = GET_ADDRESS(regs[GET_RS(instr)]);
  return 0;
}

int andi(uint32_t instr){
  regs[GET_RT(instr)] = regs[GET_RS(instr)] & ZERO_EXTEND(GET_IMM(instr));
  return 0;
}

int jal(uint32_t instr){
  // only +4 due to +4 in intrp function.
  regs[31]=PC+4;
  PC = (((PC) & MS_4B) | (GET_ADDRESS(instr) << 2));
  return 0;

}

int ori(uint32_t instr){
  regs[GET_RT(instr)] = regs[GET_RS(instr)]| ZERO_EXTEND(GET_IMM(instr));
  return 0;
}

int lui(uint32_t instr){
  regs[GET_RT(instr)] = GET_IMM(instr) << 16;
  return 0;
}

int lw(uint32_t instr){
  regs[GET_RT(instr)] = GET_BIGWORD(mem,regs[GET_RS(instr)]+SIGN_EXTEND(GET_IMM(instr)));
  return 0;
}

int sw(uint32_t instr){
  SET_BIGWORD(mem, regs[GET_RS(instr)] + SIGN_EXTEND(GET_IMM(instr)), regs[GET_RT(instr)]);
  return 0;
}

// interps alle R-functions
int intrp_r(uint32_t instr){
  switch(GET_FUNCT(instr)){
  case FUNCT_SYSCALL:
    return 1;
  case FUNCT_JR :
    return jr(instr);
  case FUNCT_ADDU :
    return addu(instr);
  case FUNCT_SUBU :
    return subu(instr);
  case FUNCT_AND :
    return and(instr);
  case FUNCT_OR :
    return or(instr);
  case FUNCT_NOR :
    return nor(instr);
  case FUNCT_SLT :
    return slt(instr);
  case FUNCT_SLL :
    return sll(instr);
  case FUNCT_SRL : 
    return srl(instr);
  default:
    return ERROR_UNKNOWN_FUNCT;
  }
}
int interp_inst(uint32_t instr){
  //gets opcode by GET_UPCODE(instr) and do swicth case 
  switch(GET_OPCODE(instr)){
  case OPCODE_R:
    return intrp_r(instr);
  case OPCODE_ADDIU:
    return addiu(instr);
  case OPCODE_BNE:
    return bne(instr);
  case OPCODE_BEQ:
    return beq(instr);
  case OPCODE_SLTI:
    return slti(instr);
  case OPCODE_ANDI:
    return andi(instr);
  case OPCODE_J:
    return j(instr);
  case OPCODE_JAL:
    return jal(instr);
  case OPCODE_ORI:
    return ori(instr);
  case OPCODE_LUI:
    return lui(instr);
  case OPCODE_LW:
    return lw(instr);
  case OPCODE_SW:
    return sw(instr);
  default : 
    return ERROR_UNKNOWN_OPCODE;
  }
}
void interp_if(){
  uint32_t addr = GET_BIGWORD(mem,PC);
  preg_if_id.inst = addr;
  PC+=4;
  instr_cnt ++;
}
int interp_control(){
  if (if_id.inst == 0){
    id_ex.mem_read = 0 ;
    id_ex.mem_write = 0;
    id_ex.reg_write = 0;
    id_ex.funct = 0;
    return 0;
  }
  switch (GET_OPCODE(if_id.inst)){
  case OPCODE_LW:
    id_ex.mem_read = 1;
    id_ex.mem_write = 0;
    id_ex.reg_write = 0;
    id_ex.funct = FUNCT_ADD;
    return 0;
  case OPCODE_SW:
    id_ex.mem.read = 0;
    id_ex.mem_write = 1;
    id_ex.reg_write = 0;
    id_ex.funct = FUNCT_ADD;
    return 0;
  default:
    return ERROR_UNKNOWN_OPCODE;
  }
}
int interp_id(){
  if_ex.rt = GET_RT(if_id.inst);
  if_ex.rs_value = regs[GET_RS(if_id.inst)];
  if_ex.rt_value = regs[rt];
  if_ex.sign_ext_imm = SIGN_EXTEND(GET_IMM(if_id.inst));
  int returnvalue = interp_control();
  if (returnvalue != 0){
    return returnvalue;
  }
  return 0;
}
int cycles(){
  int returnvalue = interp_id();
  if (returnvalue != 0){
    return returnvalue;
  }
  interp_if();
  //return SAW_SYSCALL
  return 0;
}
int interp(){
  //for-ever loop
  for (;;){
      cycles ++;
      /*int count = interp_inst(GET_BIGWORD(mem,PC)); // gets instr from PC in mem array
        if (count == 1){ //sees the syscall function;
        return 0;
        }
        if (count == ERROR_UNKNOWN_FUNCT){ // unknown Funct in R-type instruction
        return ERROR_UNKNOWN_FUNCT;
        }
        if (count == ERROR_UNKNOWN_OPCODE){ //error occured eg. unsupported function
        return ERROR_UNKNOWN_OPCODE;
        }
        PC +=4;
    }*/
      int cyclesreturn = cycles();
      if (cyclesreturn != 0) break;
  }
  if (cyclesreturn == SAW_SYSCALL){
    return 0;
  } else {
    return 1;
  }
}
int main(int argc, char *argv[]){
   if(argc != 3){
     printf("Insufficient arguments\n");
     return ERROR_INVALID_ARGS;
   }
   // checks return value of read cfg FILE.
   if(read_config(argv[1]) != 0) {
     return ERROR_IO_ERROR;
   } 
     //return value of elf_dump. 
   int elfreturn = elf_dump(argv[2], &PC, &mem[0], MEMSZ);
   if (elfreturn != 0){
     printf("elf_dump failed");
     return 1;
   } 
   // sets the stack-pointer
   regs[29] = MIPS_RESERVE+MEMSZ-4;
   // return value of interp
   int interpreturn = interp();
   if (interpreturn == 0){
     return show_status();
   }
   return interpreturn;
}



