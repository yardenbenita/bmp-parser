#include <stdio.h>
#include <stdint.h>

int main(void)
{
    uint32_t number_of_rows = 100000;
    uint32_t padded_row_size = 50000;

    /*
     * Calculate the largest number_of_rows that can safely be
     * multiplied by padded_row_size without overflowing uint32_t.
     */
    uint32_t max_safe_rows =
        UINT32_MAX / padded_row_size;

    printf("number_of_rows = %u\n", number_of_rows);
    printf("padded_row_size = %u\n", padded_row_size);
    printf("max_safe_rows = %u\n", max_safe_rows);

    /*
     * Check for overflow BEFORE performing the multiplication.
     */
    if (number_of_rows > max_safe_rows)
    {
        printf("Overflow would occur!\n");
    }
    else
    {
        printf("Multiplication is safe.\n");
    }

    /*
     * Perform the multiplication anyway so we can observe
     * the wrapped uint32_t result.
     */
    uint32_t image_size =
        number_of_rows * padded_row_size;

    printf("image_size = %u\n", image_size);

    return 0;
}