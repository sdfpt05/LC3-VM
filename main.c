#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/termios.h>
#include <signal.h>
#include <errno.h>

#define MEMORY_MAX (1 << 16)
#define MEMORY_SIZE MEMORY_MAX

// Memory
static uint16_t memory[MEMORY_SIZE];

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

static uint16_t reg[R_COUNT];

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

// Function prototypes
static void update_flags(uint16_t r);
static uint16_t sign_extend(uint16_t x, int bit_count);
static uint16_t swap16(uint16_t x);
static uint16_t check_key(void);
static void mem_write(uint16_t address, uint16_t val);
static uint16_t mem_read(uint16_t address);
static int read_image(const char *image_path);
static void handle_interrupt(int signal);
static void disable_input_buffering(void);
static void restore_input_buffering(void);

// Improved error handling
#define PRINT_ERROR(...) fprintf(stderr, "Error: " __VA_ARGS__)
#define EXIT_WITH_ERROR(...)      \
    do                            \
    {                             \
        PRINT_ERROR(__VA_ARGS__); \
        exit(1);                  \
    } while (0)

// Instruction execution functions
static void exec_add(uint16_t instr);
static void exec_and(uint16_t instr);
static void exec_br(uint16_t instr);
static void exec_jmp(uint16_t instr);
static void exec_jsr(uint16_t instr);
static void exec_ld(uint16_t instr);
static void exec_ldi(uint16_t instr);
static void exec_ldr(uint16_t instr);
static void exec_lea(uint16_t instr);
static void exec_not(uint16_t instr);
static void exec_st(uint16_t instr);
static void exec_sti(uint16_t instr);
static void exec_str(uint16_t instr);
static void exec_trap(uint16_t instr);

// Trap routines
static void trap_getc(void);
static void trap_out(void);
static void trap_puts(void);
static void trap_in(void);
static void trap_putsp(void);
static void trap_halt(void);

// Global variables for input buffering
static struct termios original_tio;
static volatile sig_atomic_t running = 1;

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        PRINT_ERROR("Usage: %s <image-file1> ...\n", argv[0]);
        exit(2);
    }

    for (int j = 1; j < argc; ++j)
    {
        if (!read_image(argv[j]))
        {
            EXIT_WITH_ERROR("Failed to load image: %s\n", argv[j]);
        }
    }

    signal(SIGINT, handle_interrupt);
    disable_input_buffering();

    // Set the PC to starting position
    // 0x3000 is the default
    enum
    {
        PC_START = 0x3000
    };
    reg[R_PC] = PC_START;

    while (running)
    {
        uint16_t instr = mem_read(reg[R_PC]++);
        uint16_t op = instr >> 12;

        switch (op)
        {
        case OP_ADD:
            exec_add(instr);
            break;
        case OP_AND:
            exec_and(instr);
            break;
        case OP_BR:
            exec_br(instr);
            break;
        case OP_JMP:
            exec_jmp(instr);
            break;
        case OP_JSR:
            exec_jsr(instr);
            break;
        case OP_LD:
            exec_ld(instr);
            break;
        case OP_LDI:
            exec_ldi(instr);
            break;
        case OP_LDR:
            exec_ldr(instr);
            break;
        case OP_LEA:
            exec_lea(instr);
            break;
        case OP_NOT:
            exec_not(instr);
            break;
        case OP_ST:
            exec_st(instr);
            break;
        case OP_STI:
            exec_sti(instr);
            break;
        case OP_STR:
            exec_str(instr);
            break;
        case OP_TRAP:
            exec_trap(instr);
            break;
        case OP_RES:
        case OP_RTI:
        default:
            PRINT_ERROR("BAD OPCODE: %d\n", op);
            running = 0;
            break;
        }
    }

    restore_input_buffering();
    return 0;
}
