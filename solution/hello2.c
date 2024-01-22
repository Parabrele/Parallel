#include <stdio.h>
#include <mpi.h>

int main(int argc, char *argv[])
{
    int rank, size;
    MPI_Init(&argc, &argv);               // Initialize MPI
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Get current process id
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Get number of processes
    if (rank == 0)
    {
        int received = 0;
        while (received < size - 1)
        {
            int buffer;
            MPI_Recv(&buffer, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("Received %d from process %d\n", buffer, buffer);
            received++;
        }
        printf("Hello world with %d ready task(s)\n", received + 1);
    }
    else
    {
        int buffer = rank;
        MPI_Send(&buffer, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
    MPI_Finalize(); // Terminate MPI
}