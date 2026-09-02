#include <stdio.h>
#include <stdint.h>

int main(void)
{
    uint32_t number_of_rows = 100000;
    uint32_t padded_row_size = 50000;

    uint32_t calculated_image_size = number_of_rows * padded_row_size;

    uint64_t real_image_size = (uint64_t)number_of_rows * padded_row_size;

    printf("number_of_rows = %u\n", number_of_rows);
    printf("padded_row_size = %u\n", padded_row_size);
    printf("calculated_image_size = %u\n", calculated_image_size);
    printf("real_image_size = %llu\n", (unsigned long long)real_image_size);

    return 0;
}