#include <stdio.h>

#include "parser.h"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <bmp_file>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "rb");

    if (file == NULL)
    {
        fprintf(stderr, "Error: failed to open file: %s\n", argv[1]);
        return 1;
    }

    int result = parse_bitmap(file);

    if (result != 0)
    {
        fclose(file);
        return 1;
    }

    if (fclose(file) != 0)
    {
        fprintf(stderr, "Error: failed to close BMP file.\n");
        return 1;
    }

    printf("BMP file parsed and validated successfully.\n");

    return 0;
}