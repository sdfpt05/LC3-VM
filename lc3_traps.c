#include <stdio.h>
#include "lc3.h"

extern uint16_t reg[R_COUNT];
extern uint16_t memory[MEMORY_MAX];
extern void update_flags(uint16_t r);

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
}