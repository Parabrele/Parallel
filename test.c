#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    // tests for difference between * and []
    int *array1;
    int array2[10];

    // let's try to assign a value to the first element of each array
    array1[0] = 1;
    array2[0] = 1;

    // let's try to print the first element of each array and the second element of each array
    printf("%d\n", array1[0]);
    printf("%d\n", array2[0]);
    printf("%d\n", array1[1]);
    printf("%d\n", array2[1]);

    //print the pointer for each array
    printf("%p\n", array1);
    printf("%p\n", array2);

    // now try to assign a new pointer
    array1 = array1 + 1;
    array2 = array2 + 1;

    printf("%d\n", array1[0]);
    printf("%d\n", array2[0]);
}