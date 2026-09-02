#include <stdio.h>

int main(void)
{
    int numbers[4] = {10, 20, 30, 40};

    printf("Valid value: %d\n", numbers[2]);
    printf("Out of bounds value: %d\n", numbers[4]);

    return 0;
}