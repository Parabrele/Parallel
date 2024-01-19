/*
RESULTS :

Implementation 1 :
one worker generates the array, then sends it to every other worker and they compute the max value on their portion of the array.
This is slower than the max1 implementation because there is still one guy generating the array, and now he does other stuff too.

Implementation 2 :
every worker generates the array and computes the max on its portion.
This is exactly as fast as max1 because the bottleneck is not computing the max, but generating the array.


If we look only at the time taken to compute the max, implementation 2 is faster than max1.
However, it relies on the fact that every one is able to recompute the array.
In a real application, the array is not computable and must be sent so only implementation 1 is interesting.
Since sending is still slower than computing the max, it would still be better not to parallelise anything.
*/

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int *generate_array(int seed, int size)
{
    // TODO : use stride and offset to generate a non contiguous array
    /*
    This function generate an array based on the seed and the size.
    */
    int i;
    int array[size];

    if (seed > -1)
        srand48(seed);

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

int fuse_max(int max, int rank, int nb_proc)
{
    if (rank == 0)
    {
        for (int i = 1; i < nb_proc; i++)
        {
            int tmp;
            MPI_Recv(&tmp, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (tmp > max)
            {
                max = tmp;
            }
        }
    }
    else
    {
        MPI_Send(&max, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
    return max;
}

int worker_v0(int seed, int N, int rank, int nb_proc)
{
    /*
    This worker computes the maximum value inside a portion of the array based on the rank of the process.
    Only the master process will generate the array and send it to the other processes.
    */
    int max;

    if (rank == 0)
    {
        float t1 = MPI_Wtime();
        int array[N];
        array = generate_array(seed, N);
        for (int i = 1; i < nb_proc; i++)
        {
            //remark : I should only send a portion of the array, but anyway this is not the bottleneck.
            //          Following the remark at the beginning of the file, this would become the bottleneck
            //          should we not have to generate the array. But anyway this would still be slower than
            //          not parallelising since one process would send the array and doing so is already slower
            //          than computing the max.
            MPI_Send(
                array + N / nb_proc * i,
                N/nb_proc,
                MPI_INT, i, 0, MPI_COMM_WORLD
            );
        }
        float t2 = MPI_Wtime();
        printf("%d Time to generate and distribute array: %f ms\n", rank, (t2 - t1) * 1000);

        float t3 = MPI_Wtime();
        max = max_array(array, N / nb_proc, 0);
        float t4 = MPI_Wtime();
        printf("%d Time to compute max: %f ms\n", rank, (t4 - t3) * 1000);
        free(array);
    }
    else
    {
        int array[N / nb_proc];
        MPI_Recv(array, N / nb_proc, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        float t3 = MPI_Wtime();
        max = max_array(array, N / nb_proc, 0);
        float t4 = MPI_Wtime();
        printf("%d Time to compute max: %f ms\n", rank, (t4 - t3) * 1000);
        free(array);
    }

    max = fuse_max(max, rank, nb_proc);

    return max;
}

int worker_v1(int seed, int N, int rank, int nb_proc)
{
    /*
    This worker generates the whole array instead of receiving it as argument to avoid large communications and compare the performance.
    */
    int max;
    int array[N];

    float t1 = MPI_Wtime();
    array = generate_array(seed, N);
    float t2 = MPI_Wtime();
    printf("%d Time to generate array: %f ms\n", rank, (t2 - t1) * 1000);
    float t3 = MPI_Wtime();
    max = max_array(array, N / nb_proc * (rank + 1), N / nb_proc * rank);
    float t4 = MPI_Wtime();
    printf("%d Time to compute max: %f ms\n", rank, (t4 - t3) * 1000);
    free(array);

    max = fuse_max(max, rank, nb_proc);

    return max;
}

int main(int argc, char **argv)
{
    int N, stride, offset;
    int max;
    int seed;
    float t1, t2;

    int TEST_VERSION = 1;

    if (argc != 3)
    {
        fprintf(stderr, "Function needs 2 arguments: seed and number of elements\n");
        exit(1);
    }

    seed = atoi(argv[1]);
    N = atoi(argv[2]);

    int rank, nb_proc;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);

    t1 = MPI_Wtime();

    // check if the number of elements is divisible by the number of processes
    if (N % nb_proc != 0)
    {
        fprintf(stderr, "Number of elements must be divisible by the number of processes\n");
        exit(1);
    }

    if (TEST_VERSION == 0)
    {
        max = worker_v0(seed, N, rank, nb_proc);
    }
    else if (TEST_VERSION == 1)
    {
        max = worker_v1(seed, N, rank, nb_proc);
        //} else if (TEST_VERSION == 2)
        //{
        //    max = worker_v2(seed, N, rank, nb_proc, array);
    }
    else
    {
        fprintf(stderr, "Wrong version number\n");
        exit(1);
    }

    t2 = MPI_Wtime();

    if (rank == 0)
    {
        printf("Max value: %d\n", max);
        printf("Time: %f ms\n", (t2 - t1) * 1000);
    }

    MPI_Finalize();
    return 0;
}