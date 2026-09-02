#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int main(void)
{
    uint16_t number_of_rows = 300;
    uint16_t padded_row_size = 300;

    uint16_t calculated_image_size = number_of_rows * padded_row_size;
    uint32_t real_image_size = (uint32_t)number_of_rows * padded_row_size;

    printf("number_of_rows = %u\n", number_of_rows);
    printf("padded_row_size = %u\n", padded_row_size);
    printf("calculated_image_size = %u\n", calculated_image_size);
    printf("real_image_size = %u\n", real_image_size);

    uint8_t *buffer = malloc(calculated_image_size);

    if (buffer == NULL)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        return 1;
    }

    printf("Allocated buffer size = %u bytes\n", calculated_image_size);

    for (uint32_t i = 0; i < real_image_size; i++)
    {
        buffer[i] = 0x41;
    }

    free(buffer);

    return 0;
}