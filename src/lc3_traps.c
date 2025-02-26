#include <stdio.h>
#include "lc3.h"

void trap_getc()
{
    reg[R_R0] = (uint16_t)getchar();
    update_flags(R_R0);
}

void trap_out()
{
    putc((char)reg[R_R0], stdout);
    fflush(stdout);
}

void trap_puts()
{
    uint16_t *c = memory + reg[R_R0];
    while (*c)
    {
        putc((char)*c, stdout);
        ++c;
    }
    fflush(stdout);
}

void trap_in()
{
    printf("Enter a character: ");
    char c = getchar();
    putc(c, stdout);
    reg[R_R0] = (uint16_t)c;
    update_flags(R_R0);
    fflush(stdout);
}

void trap_putsp()
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

void trap_halt()
{
    puts("HALT");
    fflush(stdout);
    running = 0;
}

void exec_trap(uint16_t instr)
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
        fprintf(stderr, "Error: Unknown trap code: %X\n", instr & 0xFF);
        running = 0;
        break;
    }
}