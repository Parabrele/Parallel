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
    
    if (N % nb_proc != 0)
    {
        fprintf(stderr, "Number of elements must be divisible by the number of processes\n");
        exit(1);
    }

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

        // distribute the work
        // Note :
        // I think this is not what the question wanted based on the comment on the next question.
        // I should treat one array completely before moving to the next one.
        // But here I do all the distribution of all the arrays, then all the computation, then all the collection.
        // So in the end there is the same amount of synchronisation and messages but more in a burst manner, with a lot of
        // time between the two burst during which the processes are going full BRRRRR on their own.

        // I don't know if there is any actual difference in performance between the two methods.
        // I implemented the difference between the two versions described here in max4v1.c and max4v2.c.
        // See these files for more details.
        float t3 = MPI_Wtime();
        for (int proc = 1; proc < nb_proc; proc++)
        {
            for (int arr = 0; arr < M; arr++)
            {
                MPI_Send(
                    array[arr] + proc * N / nb_proc,
                    N / nb_proc,
                    MPI_INT, proc, arr, MPI_COMM_WORLD
                );
            }
        }
        float t4 = MPI_Wtime();

        // do its part of the work
        float t5 = MPI_Wtime();
        for (int arr = 0; arr < M; arr++)
        {
            max[arr] = max_array(array[arr], N / nb_proc, 0);
            free(array[arr]);
        }
        float t6 = MPI_Wtime();

        // collect the results
        float t7 = MPI_Wtime();
        for (int proc = 1; proc < nb_proc; proc++)
        {
            for (int arr = 0; arr < M; arr++)
            {
                int tmp;
                MPI_Recv(&tmp, 1, MPI_INT, proc, arr, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                if (tmp > max[arr])
                {
                    max[arr] = tmp;
                }
            }
        }
        float t8 = MPI_Wtime();

        // print the results
        for (int arr = 0; arr < M; arr++)
        {
            printf("Max of array %d: %d\n", arr, max[arr]);
        }

        printf("Time to distribute the arrays: %f ms\n", (t4 - t3) * 1000);
        printf("Time to compute the max: %f ms\n", (t6 - t5) * 1000);
        printf("Time to generate the arrays: %f ms\n", (t2 - t1) * 1000);
        printf("Time to get the max: %f ms\n", (t8 - t7 + t6 - t5 + t4 - t3) * 1000);
        printf("Total time: %f ms\n", (t8 - t7 + t6 - t5 + t4 - t3 + t2 - t1) * 1000);
    } else {
        // receive the work
        for (int arr = 0; arr < M; arr++)
        {
            array[arr] = (int *)malloc(N / nb_proc * sizeof(int));
            MPI_Recv(array[arr], N / nb_proc, MPI_INT, 0, arr, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        // do its part of the work
        for (int arr = 0; arr < M; arr++)
        {
            max[arr] = max_array(array[arr], N / nb_proc, 0);
            free(array[arr]);
        }

        // send the result
        for (int arr = 0; arr < M; arr++)
        {
            MPI_Send(&max[arr], 1, MPI_INT, 0, arr, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    return 0;
}