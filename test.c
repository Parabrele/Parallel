#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int seed = atoi(argv[1]);
    int val = seed;
    srand48(seed);
    int i;
    for (i = 0; i < 2; i++)
    {
        val = lrand48();
        srand48(val);
        printf("%d\n", val);
    }
}