#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#include <stdint.h>

#define PCG32_DEFAULT_STATE 0x853c49e6748fea9bULL
#define PCG32_DEFAULT_STREAM 0xda3e39cb94b95bdbULL
#define PCG32_MULT 0x5851f42d4c957f2dULL

typedef struct
{
    uint64_t state;
    uint64_t inc;
} pcg32_random_t;

uint32_t pcg32_random_r(pcg32_random_t *rng)
{
    uint64_t oldstate = rng->state;
    rng->state = oldstate * PCG32_MULT + rng->inc;
    uint32_t xorshifted = (uint32_t)(((oldstate >> 18u) ^ oldstate) >> 27u);
    uint32_t rot = (uint32_t)(oldstate >> 59u);
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

void pcg32_srandom(pcg32_random_t *rng, uint64_t initstate, uint64_t initseq)
{
    rng->state = 0U;
    rng->inc = (initseq << 1u) | 1u;
    pcg32_random_r(rng);
    rng->state += initstate;
    pcg32_random_r(rng);
}

void pcg32_advance(pcg32_random_t *rng, int64_t delta)
{
    uint64_t cur_mult = PCG32_MULT;
    uint64_t cur_plus = rng->inc;
    uint64_t acc_mult = 1u;
    uint64_t acc_plus = 0u;

    uint64_t abs_delta = (delta > 0) ? delta : -delta;

    while (abs_delta > 0)
    {
        if (abs_delta & 1)
        {
            acc_mult *= cur_mult;
            acc_plus = acc_plus * cur_mult + cur_plus;
        }
        cur_plus = (cur_mult + 1) * cur_plus;
        cur_mult *= cur_mult;
        abs_delta /= 2;
    }

    rng->state = acc_mult * rng->state + acc_plus;
}

int main(int argc, char *argv[])
{
    int rank, size;
    MPI_Init(&argc, &argv); // Initialize MPI
    double start = MPI_Wtime();
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Get current process id
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Get number of processes

    // Check the number of arguments
    if (argc != 3)
    {
        printf("Usage: %s <seed> <number of elements>\n", argv[0]);
        return 1;
    }
    printf("Rank %d of %d\n", rank, size);

    // Get the arguments
    int seed = atoi(argv[1]);
    int n = atoi(argv[2]);

    // Initialize the pseudo-random number generator
    pcg32_random_t rng;
    pcg32_srandom(&rng, seed, PCG32_DEFAULT_STREAM);

    int chunk = n / size;
    // Skip the elements that are not in the current chunk
    pcg32_advance(&rng, chunk * rank);
    if (rank == size - 1)
    {
        chunk = n - rank * chunk;
    }
    // Allocate the array
    int *array = (int *)malloc(chunk * sizeof(int));

    // Initialize the array with random values
    for (int i = 0; i < chunk; i++)
    {
        array[i] = pcg32_random_r(&rng);
    }

    // Search for the maximum value
    int max = 0;
    for (int i = 0; i < chunk; i++)
    {
        if (array[i] > max)
        {
            max = array[i];
        }
    }

    if (rank == 0)
    {
        int received = 0;
        while (received < size - 1)
        {
            int buffer;
            MPI_Recv(&buffer, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (buffer > max)
            {
                max = buffer;
            }
            received++;
        }
        printf("Max value: %d\n", max);
        double end = MPI_Wtime();
        double time = end - start;
        printf("Time: %f\n", time);
    }
    else
    {
        MPI_Send(&max, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
    
    // Terminate MPI
    MPI_Finalize();
    return 0;
}