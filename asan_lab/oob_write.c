#include <stdio.h>

int main(void)
{
    int numbers[4] = {10, 20, 30, 40};

    numbers[4] = 99;

    printf("numbers[2] = %d\n", numbers[2]);

    return 0;
}