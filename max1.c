/*
In this part, we will explore one kind of parallelism that can be exploited with a distributed-memory model like MPI: the work decomposition. The main goal of this exercise is to find the maximum value inside an array with the help of parallelism.

You can rely on a program that fills an array and finds the maximum value inside based on pseudo random generator. The application max1.c takes 2 arguments:
Seed S
Number of elements N
Based on these arguments, the seed S is used to initialize the pseudo-random number generator (e.g., using the srand48 and lrand48 functions) and the number of elements N determines the size of the target array. Thus running the application multiple times with the same inputs (i.e., the same values of S and N) should produce the same result.

To evaluate the performance of the program, we need to measure its execution time. This could be done using the time command, however this would include many unecessary aspects of the program execution (job submission, data allocation and initialization of the array...). Instead, we propose to rely on the MPI_Wtime function for measuring only the portion of code that searches for the maximum value.
*/

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

// Function main takes 2 arguments:
// Seed S
// Number of elements N

//This is the sequential version of the program, used as a reference


int *generate_array(int seed, int size)
{
    //TODO : use stride and offset to generate a non contiguous array and test the performance
    /*
    This function generate an array based on the seed and the size.
    */
    int i;
    int *array;

    
    if (seed > -1) srand48(seed);

    array = (int *)malloc(size * sizeof(int));

    for (i = 0; i < size; i++)
    {
        array[i] = lrand48();
    }

    return array;
}

int max_array(int *array, int size, int offset)
{
    /*
    This function returns the maximum value inside an array.
    */
    int i;
    int max;

    max = array[offset];
    for (i = offset + 1; i < size; i += 1)
    {
        if (array[i] > max)
        {
            max = array[i];
        }
    }

    return max;
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int N;
    int *array;
    int max;
    int seed;
    
    if (argc != 3)
    {
        fprintf(stderr, "Function needs 2 arguments: seed and number of elements\n");
        exit(1);
    }
    
    seed = atoi(argv[1]);
    N = atoi(argv[2]);
    
    array = generate_array(seed, N);
    
    float t1 = MPI_Wtime();
    
    max = max_array(array, N, 0);
    
    printf("Maximum value: %d\n", max);
    
    free(array);
    
    float t2 = MPI_Wtime();
    printf("Time: %f ms\n", (t2 - t1) * 1000);
    MPI_Finalize();
    return 0;
}
