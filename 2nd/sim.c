#include <stdio.h>
#include "mips32.h"
#include <stdint.h>
#include "elf.h"
#include <stdlib.h>
#include <stdbool.h>
#define ERROR_INVALID_ARGS 1
#define ERROR_IO_ERROR 2
#define ERROR_UNKNOWN_OPCODE 3
#define ERROR_UNKNOWN_FUNCT 4
#define MEMSZ 640*1024
#define SAW_SYSCALL 5
static uint32_t PC;
static uint32_t regs[32];
static size_t instr_cnt = 0;
static unsigned char mem[MEMSZ];
static size_t cycles_cnt = 0;
struct preg_if_id {
  uint32_t inst;
};
static struct preg_if_id if_id;
struct preg_id_ex {
  bool alu_src;
  bool mem_read;
  bool mem_write;
  bool reg_write;
  uint32_t rt; // = GET_RT(if_id.inst);
  uint32_t rs_value;// = regs[GET_RS(if_id.inst)];
  uint32_t rt_value;// = regs[rt];
  uint32_t sign_ext_imm; //= SIGN_EXTEND(GET_IMM(if_id.inst));
  uint32_t funct;
};
static struct preg_id_ex id_ex;
struct preg_ex_mem {
  bool mem_read;
  bool mem_write;
  bool reg_write;
  uint32_t rt;// = GET_RT(if_id.inst);
  uint32_t rt_value;// = regs[rt];
  int alu_res; //result of alu()
};
static struct preg_ex_mem ex_mem;
struct preg_mem_wb {
  bool reg_write;
  uint32_t rt;
  uint32_t read_data;
  int alu_res; // result of alu()
};
static struct preg_mem_wb mem_wb;
int show_status(){
  printf("Executed %zu instruction(s).\n", instr_cnt); 
  printf("Executed %zu cycle(s).\n", cycles_cnt);
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
  for (int i = 8; i < 16; i++){
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
void show_regs_status(){
  printf("cycle nr. = %zu\n",instr_cnt);
  printf("VALUES OF IF/ID\n");
  printf("value of IF/ID.inst = %x\n",if_id.inst);
  printf("VALUES OF ID/EX\n");
  printf("value of ID/EX.mem_read = %d\n",id_ex.mem_read);
  printf("value of ID/EX.mem_write = %d\n", id_ex.mem_write);
  printf("value of ID/EX.reg_write = %d\n", id_ex.reg_write);
  printf("value of ID/EX.rt = %x\n",id_ex.rt);
  printf("value of ID/EX.rs = %x\n",id_ex.rs_value);
  printf("value of ID/EX.rt_value = %x\n",id_ex.rt_value);
  printf("value of ID/EX.sign_ext_imm = %x\n", id_ex.sign_ext_imm);
  printf("value of ID/EX.funct = %x\n",id_ex.funct);
  printf("VALUES OF EX/MEM\n");  
  printf("value of EX/MEM.mem_read = %d\n", ex_mem.mem_read);
  printf("value of EX/MEM.mem_write = %d\n", ex_mem.mem_write);
  printf("value of EX/MEM.reg_write = %d\n", ex_mem.reg_write);
  printf("value of EX/MEM.rt = %x\n",ex_mem.rt);
  printf("value of EX/MEM.rt_value = %x\n",ex_mem.rt_value);
  printf("value of EX/MEM.alu_res = %x\n",ex_mem.alu_res);
  printf("END OF CYCLE\n");
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
  PC = regs[GET_RS(instr)];
  return 0;
}

int andi(uint32_t instr){
  regs[GET_RT(instr)] = regs[GET_RS(instr)] & ZERO_EXTEND(GET_IMM(instr));
  return 0;
}

int jal(uint32_t instr){
  // only +4 due to +4 in intrp function.
  regs[31]=PC;
  PC = (((PC+4) & MS_4B) | (GET_ADDRESS(instr) << 2));
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
void interp_wb(){
  if (mem_wb.reg_write == 1){
    if (mem_wb.rt != 0){
      regs[mem_wb.rt]=mem_wb.read_data;
    }
  }
}
void interp_mem(){
  mem_wb.reg_write = ex_mem.reg_write;
  mem_wb.rt = ex_mem.rt;
  mem_wb.alu_src = ex_mem.alu_src;
  if (ex_mem.mem_read == 1){
    mem_wb.read_data = GET_BIGWORD(mem,ex_mem.alu_res);
  }
  if (ex_mem.mem_write == 1) {
    mem_wb.read_data = SET_BIGWORD(mem,ex_mem.alu_res,ex_mem.rt_value);
  }
  
}
void interp_if(){
  uint32_t addr = GET_BIGWORD(mem,PC);
  if_id.inst = addr;
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
    id_ex.alu_src = 1;
    return 0;
  case OPCODE_SW:
    id_ex.mem_read = 0;
    id_ex.mem_write = 1;
    id_ex.reg_write = 0;
    id_ex.funct = FUNCT_ADD;
    id_ex.alu_src = 1;
    return 0;
  case OPCODE_R:
    id_ex.alu_src = 0;
    id_ex.mem_read = 0;
    id_ex.mem_write = 0;
    id_ex.reg_write = 1;
    id_ex.funct = GET_FUNCT(if_id.inst);
    return 0;
  default:
    return ERROR_UNKNOWN_OPCODE;
  }
}
int alu(){
  if (id_ex.alu_src == 1) {
    
  }
  switch(id_ex.funct){
  case FUNCT_ADD:
    ex_mem.alu_res = id_ex.sign_ext_imm + id_ex.rs_value;
    return 0;
  case 0:
    return 0;
  default:
    return ERROR_UNKNOWN_FUNCT;
  }
  
}
int interp_ex(){
  ex_mem.mem_read = id_ex.mem_read;
  ex_mem.mem_write = id_ex.mem_write;
  ex_mem.reg_write = id_ex.reg_write;
  int alureturn = alu();
  if (alureturn != 0) {
    return alureturn;
  }
  if (GET_FUNCT(if_id.inst) == FUNCT_SYSCALL) {
    return SAW_SYSCALL;
  }
  return 0;
}
int interp_id(){
  id_ex.rt = GET_RT(if_id.inst);
  id_ex.rs_value = regs[GET_RS(if_id.inst)];
  id_ex.rt_value = regs[id_ex.rt];
  id_ex.sign_ext_imm = SIGN_EXTEND(GET_IMM(if_id.inst));
  int returnvalue = interp_control();
  if (returnvalue != 0){
    return returnvalue;
  }
  return 0;
}
int cycle(){
  interp_wb();
  interp_mem();
  int returnvalue = interp_ex();
  if (returnvalue != 0) {
    return returnvalue;
  }
  returnvalue = interp_id();
  if (returnvalue != 0){
    return returnvalue;
  }
  interp_if();
  show_regs_status();
  return 0;
}
int interp(){
  //for-ever loop
  int cyclesreturn;
  for (;;){
    cycles_cnt ++;
    cyclesreturn = cycle();
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
   regs[29] = MIPS_RESERVE+MEMSZ;
   // return value of interp
   int interpreturn = interp();
   if (interpreturn == 0){
     return show_status();
   }
   return interpreturn;
}



