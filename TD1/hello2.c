#include <stdio.h>
#include <mpi.h>

int main(int argc, char *argv[])
{
    int rank, size;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // put current process id in rank
    MPI_Comm_size(MPI_COMM_WORLD, &size); // put total number of processes in size

    if (rank == 0)
    {
        for (int i = 1; i < size; i++)
        {
            MPI_Recv(
                NULL, // buffer address
                0,   // maximum number of elements in buffer
                MPI_INT, // type of elements in buffer
                i, // rank of source
                MPI_ANY_TAG, // message tag
                MPI_COMM_WORLD, // communicator
                &status // status object
            );
        }
        printf("Hello World with %d ready task(s)\n", size);
    }
    else
    {
        MPI_Send(NULL, 0, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}