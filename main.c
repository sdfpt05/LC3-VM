#include <stdio.h>
#include <stdlib.h>
#include "lc3.h"

#define PRINT_ERROR(...) fprintf(stderr, "Error: " __VA_ARGS__)
#define EXIT_WITH_ERROR(...)      \
    do                            \
    {                             \
        PRINT_ERROR(__VA_ARGS__); \
        exit(1);                  \
    } while (0)

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        PRINT_ERROR("Usage: %s <image-file1> ...\n", argv[0]);
        exit(2);
    }

    lc3_init();

    for (int j = 1; j < argc; ++j)
    {
        if (!lc3_load_image(argv[j]))
        {
            EXIT_WITH_ERROR("Failed to load image: %s\n", argv[j]);
        }
    }

    lc3_run();
    lc3_cleanup();

    return 0;
}