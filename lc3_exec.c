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

static void update_flags(uint16_t r)
{
    if (reg[r] == 0)
    {
        reg[R_COND] = FL_ZRO;
    }
    else if (reg[r] >> 15)
    {
        reg[R_COND] = FL_NEG;
    }
    else
    {
        reg[R_COND] = FL_POS;
    }
}

static uint16_t sign_extend(uint16_t x, int bit_count)
{
    if ((x >> (bit_count - 1)) & 1)
    {
        x |= (0xFFFF << bit_count);
    }
    return x;
}

static uint16_t swap16(uint16_t x)
{
    return (x << 8) | (x >> 8);
}

static uint16_t check_key()
{
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    return select(1, &readfds, NULL, NULL, &timeout) != 0;
}

static void mem_write(uint16_t address, uint16_t val)
{
    memory[address] = val;
}

static uint16_t mem_read(uint16_t address)
{
    if (address == MR_KBSR)
    {
        if (check_key())
        {
            memory[MR_KBSR] = (1 << 15);
            memory[MR_KBDR] = getchar();
        }
        else
        {
            memory[MR_KBSR] = 0;
        }
    }
    return memory[address];
}

static int read_image(const char *image_path)
{
    FILE *file = fopen(image_path, "rb");
    if (!file)
    {
        PRINT_ERROR("Could not open file: %s\n", image_path);
        return 0;
    }

    uint16_t origin;
    fread(&origin, sizeof(origin), 1, file);
    origin = swap16(origin);

    uint16_t max_read = MEMORY_MAX - origin;
    uint16_t *p = memory + origin;
    size_t read = fread(p, sizeof(uint16_t), max_read, file);

    for (size_t i = 0; i < read; ++i)
    {
        p[i] = swap16(p[i]);
    }

    fclose(file);
    return 1;
}

static void handle_interrupt(int signal)
{
    restore_input_buffering();
    printf("\n");
    exit(-2);
}

static void disable_input_buffering()
{
    tcgetattr(STDIN_FILENO, &original_tio);
    struct termios new_tio = original_tio;
    new_tio.c_lflag &= ~ICANON & ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
}

static void restore_input_buffering()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
}

// Instruction execution functions
static void exec_add(uint16_t instr)
{
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t r1 = (instr >> 6) & 0x7;
    uint16_t imm_flag = (instr >> 5) & 0x1;

    if (imm_flag)
    {
        uint16_t imm5 = sign_extend(instr & 0x1F, 5);
        reg[r0] = reg[r1] + imm5;
    }
    else
    {
        uint16_t r2 = instr & 0x7;
        reg[r0] = reg[r1] + reg[r2];
    }

    update_flags(r0);
}

static void exec_and(uint16_t instr)
{
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t r1 = (instr >> 6) & 0x7;
    uint16_t imm_flag = (instr >> 5) & 0x1;

    if (imm_flag)
    {
        uint16_t imm5 = sign_extend(instr & 0x1F, 5);
        reg[r0] = reg[r1] & imm5;
    }
    else
    {
        uint16_t r2 = instr & 0x7;
        reg[r0] = reg[r1] & reg[r2];
    }

    update_flags(r0);
}

static void exec_br(uint16_t instr)
{
    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
    uint16_t cond_flag = (instr >> 9) & 0x7;
    if (cond_flag & reg[R_COND])
    {
        reg[R_PC] += pc_offset;
    }
}

static void exec_jmp(uint16_t instr)
{
    uint16_t r1 = (instr >> 6) & 0x7;
    reg[R_PC] = reg[r1];
}

static void exec_jsr(uint16_t instr)
{
    uint16_t long_flag = (instr >> 11) & 1;
    reg[R_R7] = reg[R_PC];
    if (long_flag)
    {
        uint16_t long_pc_offset = sign_extend(instr & 0x7FF, 11);
        reg[R_PC] += long_pc_offset;
    }
    else
    {
        uint16_t r1 = (instr >> 6) & 0x7;
        reg[R_PC] = reg[r1];
    }
}

static void exec_ld(uint16_t instr)
{
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
    reg[r0] = mem_read(reg[R_PC] + pc_offset);
    update_flags(r0);
}

static void exec_ldi(uint16_t instr)
{
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
    reg[r0] = mem_read(mem_read(reg[R_PC] + pc_offset));
    update_flags(r0);
}

static void exec_ldr(uint16_t instr)
{
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t r1 = (instr >> 6) & 0x7;
    uint16_t offset = sign_extend(instr & 0x3F, 6);
    reg[r0] = mem_read(reg[r1] + offset);
    update_flags(r0);
}

static void exec_lea(uint16_t instr)
{
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
    reg[r0] = reg[R_PC] + pc_offset;
    update_flags(r0);
}

static void exec_not(uint16_t instr)
{
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t r1 = (instr >> 6) & 0x7;
    reg[r0] = ~reg[r1];
    update_flags(r0);
}

static void exec_st(uint16_t instr)
{
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
    mem_write(reg[R_PC] + pc_offset, reg[r0]);
}

static void exec_sti(uint16_t instr)
{
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
    mem_write(mem_read(reg[R_PC] + pc_offset), reg[r0]);
}

static void exec_str(uint16_t instr)
{
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t r1 = (instr >> 6) & 0x7;
    uint16_t offset = sign_extend(instr & 0x3F, 6);
    mem_write(reg[r1] + offset, reg[r0]);
}

static void exec_trap(uint16_t instr)
{
    switch (instr & 0xFF)
    {
    case TRAP_GETC:
        trap_getc();
        break;
    case TRAP_OUT:
        trap_out();
        break;
    case TRAP_PUTS:
        trap_puts();
        break;
    case TRAP_IN:
        trap_in();
        break;
    case TRAP_PUTSP:
        trap_putsp();
        break;
    case TRAP_HALT:
        trap_halt();
        break;
    default:
        PRINT_ERROR("Unknown trap code: %X\n", instr & 0xFF);
        running = 0;
        break;
    }
}

// Trap Routines
static void trap_getc(void)
{
    reg[R_R0] = (uint16_t)getchar();
    update_flags(R_R0);
}

static void trap_out(void)
{
    putc((char)reg[R_R0], stdout);
    fflush(stdout);
}

static void trap_puts(void)
{
    uint16_t *c = memory + reg[R_R0];
    while (*c)
    {
        putc((char)*c, stdout);
        ++c;
    }
    fflush(stdout);
}

static void trap_in(void)
{
    printf("Enter a character: ");
    char c = getchar();
    putc(c, stdout);
    reg[R_R0] = (uint16_t)c;
    update_flags(R_R0);
    fflush(stdout);
}

static void trap_putsp(void)
{
    uint16_t *c = memory + reg[R_R0];
    while (*c)
    {
        char char1 = (*c) & 0xFF;
        putc(char1, stdout);
        char char2 = (*c) >> 8;
        if (char2)
            putc(char2, stdout);
        ++c;
    }
    fflush(stdout);
}

static void trap_halt(void)
{
    puts("HALT");
    fflush(stdout);
    running = 0;
}

// Helper function to dump memory contents (for debugging)
static void memory_dump(uint16_t start, uint16_t count)
{
    for (uint16_t i = 0; i < count; i++)
    {
        if (i % 8 == 0)
        {
            printf("\n%04X: ", start + i);
        }
        printf("%04X ", memory[start + i]);
    }
    printf("\n");
}

// Helper function to dump register contents (for debugging)
static void register_dump(void)
{
    for (int i = 0; i < R_COUNT; i++)
    {
        printf("R%d: %04X ", i, reg[i]);
        if (i % 3 == 2)
            printf("\n");
    }
    printf("\n");
}

// Main execution loop (alternative implementation)
static void execute(void)
{
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

        // register_dump();
        // memory_dump(PC_START, 16);
    }
}

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

    execute();

    restore_input_buffering();
    return 0;
}