
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

d) Dynamic Work Distribution
Even if the previous work distribution approach is of interest, it might be too static to improve the performance. Indeed, the work is assigned to a predetermined MPI process, without taking into account the load inbalance that might happen during the application execution. To tackle this issue, one solution is to rely on a dynamic scheduling of array distribution. The master MPI task (rank 0 in our code) will be in charge to assign the work to one of the ready other MPI process. Of course, at the very beginning, everyone is ready, but when a task is searching for a max, it is considered as busy until the job is done.

Let's implement this solution inside a file named max5.c and compare it to the previous approach of work decomposition.
*/

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int *generate_array(int seed, int size, int array_index)
{
    //TODO : use stride and offset to generate a non contiguous array and test the performance
    /*
    This function generate an array based on the seed and the size.
    */
    int i;
    int *array;

    
    if (seed > -1) srand48(seed);

    array = (int *)malloc((size+1) * sizeof(int));

    array[0] = array_index;
    for (i = 1; i < size+1; i++)
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

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);

    if (rank==0)
    {
        int *array[M];
        int max[M];

        // generate the arrays
        float t1 = MPI_Wtime();
        srand48(seed);
        for (int i = 0; i < M; i++)
        {
            array[i] = generate_array(-1, N, i);
        }
        float t2 = MPI_Wtime();

        /*
        Process 0 will be in charge of distributing the work to the other MPI processes dynamically.
        */
        
        float t3 = MPI_Wtime();

        int initially_idle = 1;
        int dest = initially_idle;

        int nb_received = 0;
        // send the messages to ready processes
        for (int i = 0; i < M; i++)
        {
            if (initially_idle < nb_proc)
            {
                dest = initially_idle;
                initially_idle++;
            } else {
                int message[3];
                MPI_Status status;
                MPI_Recv(message, 2, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
                max[message[0]] = message[1];
                nb_received++;
                dest = status.MPI_SOURCE;
            }
            MPI_Send(array[i], N+1, MPI_INT, dest, 0, MPI_COMM_WORLD);
            free(array[i]);
        }

        // receive the last messages
        for (int i = nb_received; i < M; i++)
        {
            int message[2];
            MPI_Recv(message, 2, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            max[message[0]] = message[1];
        }

        // kill the processes
        int *kill_array = (int *)malloc((N+1) * sizeof(int));
        kill_array[0] = -1;
        for (int i = 1; i < nb_proc; i++)
        {
            MPI_Send(kill_array, N+1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
        free(kill_array);

        float t4 = MPI_Wtime();

        // print the results
        for (int i = 0; i < M; i++)
        {
            printf("Max of array %d is %d\n", i, max[i]);
        }
        printf("Time to generate the arrays: %f ms\n", (t2 - t1) * 1000);
        printf("Time to get the max: %f ms\n", (t4 - t3) * 1000);
        printf("Total time: %f ms\n", (t4 - t3 + t2 - t1) * 1000);
    } else {
        int c = 1;
        while (c)
        {
            int *array = (int *)malloc((N+1) * sizeof(int));
            MPI_Recv(array, N+1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (array[0] == -1)
            {
                c = 0;
            } else {
                // offset is 1 because the first element of the array is the index of the array
                int max = max_array(array, N+1, 1);
                int message[2] = {array[0], max};
                MPI_Send(message, 2, MPI_INT, 0, 0, MPI_COMM_WORLD);
            }
            free(array);
        }
    }

    MPI_Finalize();
}
