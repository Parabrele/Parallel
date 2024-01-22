#include <stdio.h>
#include <mpi.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int rank, size;
    // Get the hostname
    char hostname[1024];
    gethostname(hostname, 1024);
    MPI_Init(&argc, &argv);               // Initialize MPI
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Get current process id
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Get number of processes
    printf("Hello world from process %d of %d from %s\n", rank, size, hostname);
    MPI_Finalize(); // Terminate MPI
    return 0;
}