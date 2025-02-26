#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/termios.h>
#include <sys/mman.h>
#include "lc3.h"

// Global variables
uint16_t memory[MEMORY_MAX];
uint16_t reg[R_COUNT];
volatile sig_atomic_t running = 1;
static struct termios original_tio;

uint16_t sign_extend(uint16_t x, int bit_count)
{
    if ((x >> (bit_count - 1)) & 1)
    {
        x |= (0xFFFF << bit_count);
    }
    return x;
}

uint16_t swap16(uint16_t x)
{
    return (x << 8) | (x >> 8);
}

void update_flags(uint16_t r)
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

void mem_write(uint16_t address, uint16_t val)
{
    memory[address] = val;
}

uint16_t mem_read(uint16_t address)
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

void handle_interrupt(int signal)
{
    restore_input_buffering();
    printf("\n");
    exit(-2);
}

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

void lc3_init()
{
    // Clear memory and registers
    for (int i = 0; i < MEMORY_MAX; i++) {
        memory[i] = 0;
    }
    
    for (int i = 0; i < R_COUNT; i++) {
        reg[i] = 0;
    }
    
    // Setup terminal and signal handling
    signal(SIGINT, handle_interrupt);
    disable_input_buffering();
    
    // Reset running flag
    running = 1;
}

void lc3_cleanup()
{
    restore_input_buffering();
}

int lc3_load_image(const char *image_path)
{
    FILE *file = fopen(image_path, "rb");
    if (!file)
    {
        fprintf(stderr, "Error: Could not open file: %s\n", image_path);
        return 0;
    }

    uint16_t origin;
    size_t read_origin = fread(&origin, sizeof(origin), 1, file);
    if (read_origin != 1) {
        fprintf(stderr, "Error: Could not read origin from file: %s\n", image_path);
        fclose(file);
        return 0;
    }
    
    origin = swap16(origin);

    uint16_t max_read = MEMORY_MAX - origin;
    uint16_t *p = memory + origin;
    size_t read = fread(p, sizeof(uint16_t), max_read, file);

    if (read == 0) {
        fprintf(stderr, "Error: Could not read data from file: %s\n", image_path);
        fclose(file);
        return 0;
    }

    for (size_t i = 0; i < read; ++i)
    {
        p[i] = swap16(p[i]);
    }

    fclose(file);
    return 1;
}

void memory_dump(uint16_t start, uint16_t count)
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

void register_dump(void)
{
    printf("Registers:\n");
    for (int i = 0; i < R_COUNT - 2; i++)
    {
        printf("R%d: 0x%04X ", i, reg[i]);
        if (i % 3 == 2)
            printf("\n");
    }
    printf("\nPC: 0x%04X  COND: 0x%04X\n", reg[R_PC], reg[R_COND]);
}

void lc3_run()
{
    // Set the PC to starting position
    // 0x3000 is the default
    enum
    {
        PC_START = 0x3000
    };
    reg[R_PC] = PC_START;

    while (running)
    {
        // FETCH
        uint16_t instr = mem_read(reg[R_PC]++);
        uint16_t op = instr >> 12;

        // EXECUTE
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
            fprintf(stderr, "Error: BAD OPCODE: %d\n", op);
            running = 0;
            break;
        }
    }
}
