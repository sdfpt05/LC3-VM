#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lc3.h"

#define PRINT_ERROR(...) fprintf(stderr, "Error: " __VA_ARGS__)
#define EXIT_WITH_ERROR(...)      \
    do                            \
    {                             \
        PRINT_ERROR(__VA_ARGS__); \
        exit(1);                  \
    } while (0)

void print_help(const char *program_name) {
    printf("LC-3 Virtual Machine\n");
    printf("Usage: %s [options] <image-file1> [image-file2 ...]\n\n", program_name);
    printf("Options:\n");
    printf("  -h, --help     Display this help message\n");
    printf("  -d, --debug    Enable debug mode (dumps registers after each instruction)\n");
    printf("  -m, --memory   Dump memory before starting execution\n");
    printf("\n");
}

int main(int argc, char *argv[])
{
    int debug_mode = 0;
    int memory_dump_flag = 0;
    int arg_index = 1;
    
    // Parse command line arguments
    while (arg_index < argc && argv[arg_index][0] == '-') {
        if (strcmp(argv[arg_index], "-h") == 0 || 
            strcmp(argv[arg_index], "--help") == 0) {
            print_help(argv[0]);
            return 0;
        } else if (strcmp(argv[arg_index], "-d") == 0 || 
                   strcmp(argv[arg_index], "--debug") == 0) {
            debug_mode = 1;
        } else if (strcmp(argv[arg_index], "-m") == 0 || 
                   strcmp(argv[arg_index], "--memory") == 0) {
            memory_dump_flag = 1;
        } else {
            PRINT_ERROR("Unknown option: %s\n", argv[arg_index]);
            print_help(argv[0]);
            return 1;
        }
        arg_index++;
    }
    
    // Ensure at least one image file is specified
    if (arg_index >= argc)
    {
        PRINT_ERROR("No image files specified\n");
        print_help(argv[0]);
        return 2;
    }

    // Initialize the LC-3 VM
    lc3_init();

    // Load all specified image files
    for (int j = arg_index; j < argc; ++j)
    {
        printf("Loading image: %s\n", argv[j]);
        if (!lc3_load_image(argv[j]))
        {
            EXIT_WITH_ERROR("Failed to load image: %s\n", argv[j]);
        }
    }

    // Dump memory if requested
    if (memory_dump_flag) {
        printf("\nMemory Dump (0x3000 - 0x3100):\n");
        memory_dump(0x3000, 0x100);
    }
    
    // Run the VM
    if (debug_mode) {
        // If in debug mode, we'd implement step-by-step execution here
        printf("\nStarting execution in debug mode...\n");
        // A real debug mode would modify lc3_run() to step through instructions
    } else {
        printf("\nStarting execution...\n");
    }
    
    lc3_run();
    
    // Cleanup and exit
    printf("\nExecution completed.\n");
    lc3_cleanup();

    return 0;
}