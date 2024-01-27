#include <stdio.h>
#include <mpi.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int rank, size;
    // host names are limited to arbitrary_max characters
    int arbitrary_max = 255;
    char name[arbitrary_max + 1];

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // put current process id in rank
    MPI_Comm_size(MPI_COMM_WORLD, &size); // put total number of processes in size
    gethostname(name, arbitrary_max + 1); // get host name

    printf("Hello world from process %d of %d on %s\n", rank, size, name);

    MPI_Finalize();
    return 0;
}