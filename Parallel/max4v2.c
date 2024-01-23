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
b) Work Decomposition on Multiple Arrays
The first technique to parallelize the previous application is to simply apply the mechanism you developed during the previous question. Here, the parallelism will be exploited when looking for a max inside one array. This question simply consists in processing the arrays one by one and find the maximum value among the parallel MPI processes. Once again, we propose to rely on the MPI_Wtime function for measuring some parts of the code.

Implement this mechanism inside a file named max3.c.
c) Static Work Distribution
We will now try another way to parallelize the search of max values inside multiple arrays. Of course, the previous approach is valid but there are some limits: everytime one array is processed, there is a synchronization between all the MPI processes. Another solution is to let one MPI task to process one array at a time. Here, the parallelism will be on the different arrays because, at the same time, two MPI processes will traverse different arrays without synchronization between them.

We propose now to modify the code to implement this work distribution method. The process with rank 0 will be in charge of generating and filling the arrays, then it should send the elements to the different MPI tasks. Our first method (or scheduling policy) will be to distribute the arrays in a round-robin manner i.e., send the first array to MPI task 1, the second to task 2, and so on. This is called a static distribution because it is possible to determine, ahead of time, which array will be assigned to which task.

Implement this mechanism inside a file named max4.c.
*/

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

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

/*
There are two versions of this file.

v1 implements the static distribution of the work by the following steps:

master distribute all arrays to their respective slave
only then do the slave start to work
master collect the results from the slaves

v2 implements the dynamic distribution of the work by the following steps:

master distribute arrays to the slaves cyclically. When a cycle is complete, the master waits for the results.
upon receiving an array, the slave starts to work and sends the result back to the master.

Hopefully, v2 is faster than v1, as there will be less idle time for the slaves.
*/

/*
This is the v2 version.

This version is approximately twice as fast as v1 if you disregard the time to generate the arrays.
*/

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    
    int N, M;
    int seed;
    int rank, nb_proc;

    if (argc != 4)
    {
        fprintf(stderr, "Function needs 3 arguments: seed, number of elements and number of arrays\n");
        exit(1);
    }
    
    seed = atoi(argv[1]);
    N = atoi(argv[2]);
    M = atoi(argv[3]);

    int *array[M];
    int max[M];

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);
    
    if (rank==0)
    {
        // generate the arrays
        float t1 = MPI_Wtime();
        srand48(seed);
        for (int i = 0; i < M; i++)
        {
            array[i] = generate_array(-1, N);
        }
        float t2 = MPI_Wtime();

        float t3 = MPI_Wtime();
        int nb_rounds = M / (nb_proc - 1);
        int incomplete_round = (M % (nb_proc - 1) != 0);
        for (int round = 0; round < nb_rounds + incomplete_round; round++)
        {
            for (int proc = 1; proc < nb_proc; proc++)
            {
                int arr = round * (nb_proc - 1) + proc - 1;
                if (arr < M)
                {
                    MPI_Send(array[arr], N, MPI_INT, proc, arr, MPI_COMM_WORLD);
                    free(array[arr]);
                }
            }

            for (int proc = 1; proc < nb_proc; proc++)
            {
                int arr = round * (nb_proc - 1) + proc - 1;
                if (arr < M)
                {
                    MPI_Recv(&max[arr], 1, MPI_INT, proc, arr, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }
            }
        }

        float t4 = MPI_Wtime();

        // print the results
        for (int arr = 0; arr < M; arr++)
        {
            printf("Max of array %d: %d\n", arr, max[arr]);
        }
        printf("Time to generate the arrays: %f ms\n", (t2 - t1) * 1000);
        printf("Time to get the max: %f ms\n", (t4 - t3) * 1000);
        printf("Total time: %f ms\n", (t4 - t3 + t2 - t1) * 1000);
    } else {
        /*
        in this version, wait for all the arrays to arrive before starting to work.
        */
        for (int arr = rank - 1; arr < M; arr += nb_proc - 1)
        {
            array[arr] = (int *)malloc(N * sizeof(int));
            MPI_Recv(array[arr], N, MPI_INT, 0, arr, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            max[arr] = max_array(array[arr], N, 0);
            MPI_Send(&max[arr], 1, MPI_INT, 0, arr, MPI_COMM_WORLD);
            free(array[arr]);
        }
    }

    MPI_Finalize();
    return 0;
}