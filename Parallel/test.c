#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    // tests for difference between * and []
    int *array[10];

    array[0] = (int *)malloc(5 * sizeof(int));
    array[0][0] = 1;

    printf("array: %p\n", array);
    printf("array[0]: %p\n", array[0]);
    printf("array[0][0]: %p\n", array[0][0]);
}