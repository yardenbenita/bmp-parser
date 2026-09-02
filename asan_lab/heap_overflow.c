#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int *numbers = malloc(4 * sizeof(int));

    if (numbers == NULL)
    {
        return 1;
    }

    numbers[0] = 10;
    numbers[1] = 20;
    numbers[2] = 30;
    numbers[3] = 40;
    numbers[4] = 99;

    printf("numbers[3] = %d\n", numbers[3]);

    free(numbers);

    return 0;
}