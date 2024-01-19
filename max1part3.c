/*
Part 3: Work Distribution
The previous part described one well-known paradigm of parallelism with MPI: domain decomposition. The work and data are split among the MPI tasks and each process is in charge of performing the computation on a specific set of data. It might requires some additional synchronization but this is very useful though.

Another way to parallelize an application is called the Client/Server approach. This might be used in Big Data applications or web server. It is very useful when the work load may vary during the application execution because new work may come in a burst manner. In this context, one MPI task is in charge of collecting the new work to perform and to distribute the work among the available MPI tasks. The notion of available task is very important to reach high performance.

a) Max on Multiple Arrays
First of all, we propose to update the sequential code developed previously (file max1.c) to allow multiple arrays to be generated. Indeed, now, the code will take as input 3 arguments:
Seed S
Number of elements per array N
Number of arrays M
This application will first generate M arrays of N elements each based on the seed S to initialize the pseudo-random number generator. Then the code will find and print the max of each array by processing the array one by one. Once again, this code should have a deterministic behavior and give the same result for the same set of input parameters.
*/

// Function main takes 3 arguments:
// Seed S
// Number of elements N per array
// Number of arrays M

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
    float t1 = MPI_Wtime();

    int N;
    int M;
    int max;
    int seed;
    
    if (argc != 4)
    {
        fprintf(stderr, "Function needs 3 arguments: seed and number of elements\n");
        exit(1);
    }
    
    seed = atoi(argv[1]);
    N = atoi(argv[2]);
    M = atoi(argv[3]);

    int array[M][N];
    
    for (int i = 0; i < M; i++)
    {
        array[i] = generate_array(seed, N);
    }
    
    for (int i = 0; i < M; i++)
    {
        max = max_array(array[i], N, 0);
        printf("Maximum value of array %d: %d\n", i, max);

        free(array[i])
    }
    
    float t2 = MPI_Wtime();
    printf("Time: %f ms\n", (t2 - t1) * 1000);
    MPI_Finalize();
    return 0;
}
