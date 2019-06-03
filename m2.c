#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int i, num_procs, myid, total;

    MPI_Status stat;
    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    i = myid;
    total = 0;

    if ( myid == 0 ) {
        for (i = 1; i < num_procs; i++)
        {

            MPI_Recv(&i, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &stat);

            total += i;
        }
        printf("total %d\n", total);
    } else {
        MPI_Send(&i, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
}
