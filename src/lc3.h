#ifndef LC3_H
#define LC3_H

#include <stdint.h>
#include <signal.h>

#define MEMORY_MAX (1 << 16)

// Registers
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
    R_PC,
    R_COND,
    R_COUNT
};

// Opcodes
enum
{
    OP_BR = 0,
    OP_ADD,
    OP_LD,
    OP_ST,
    OP_JSR,
    OP_AND,
    OP_LDR,
    OP_STR,
    OP_RTI,
    OP_NOT,
    OP_LDI,
    OP_STI,
    OP_JMP,
    OP_RES,
    OP_LEA,
    OP_TRAP
};

// Condition Flags
enum
{
    FL_POS = 1 << 0,
    FL_ZRO = 1 << 1,
    FL_NEG = 1 << 2,
};

// Trap Codes
enum
{
    TRAP_GETC = 0x20,
    TRAP_OUT = 0x21,
    TRAP_PUTS = 0x22,
    TRAP_IN = 0x23,
    TRAP_PUTSP = 0x24,
    TRAP_HALT = 0x25
};

// Memory Mapped Registers
enum
{
    MR_KBSR = 0xFE00,
    MR_KBDR = 0xFE02
};

// External global variables
extern uint16_t memory[MEMORY_MAX];
extern uint16_t reg[R_COUNT];
extern volatile sig_atomic_t running;

// Core VM functions
void lc3_init(void);
int lc3_load_image(const char *image_path);
void lc3_run(void);
void lc3_cleanup(void);

// Utility functions
uint16_t sign_extend(uint16_t x, int bit_count);
uint16_t swap16(uint16_t x);
uint16_t mem_read(uint16_t address);
void mem_write(uint16_t address, uint16_t val);
void update_flags(uint16_t r);
void memory_dump(uint16_t start, uint16_t count);
void register_dump(void);

// Instruction execution functions
void exec_add(uint16_t instr);
void exec_and(uint16_t instr);
void exec_br(uint16_t instr);
void exec_jmp(uint16_t instr);
void exec_jsr(uint16_t instr);
void exec_ld(uint16_t instr);
void exec_ldi(uint16_t instr);
void exec_ldr(uint16_t instr);
void exec_lea(uint16_t instr);
void exec_not(uint16_t instr);
void exec_st(uint16_t instr);
void exec_sti(uint16_t instr);
void exec_str(uint16_t instr);
void exec_trap(uint16_t instr);

// Trap routines
void trap_getc(void);
void trap_out(void);
void trap_puts(void);
void trap_in(void);
void trap_putsp(void);
void trap_halt(void);

#endif // LC3_H