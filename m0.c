#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <complex.h>
#include <unistd.h>
#include <math.h>

typedef struct Position {
    int y1;
    int y2;
    int x1;
    int x2;
} Position;

int main(int argc, char *argv[])
{
    int world_size, world_rank;
    int height = 40;
    int width = 80;

    MPI_Status stat;
    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int num_per_height = floor(height / world_size);
    int num_per_width = floor(width / world_size);

    Position pos = {
        .y1 = world_rank * num_per_height,
        .y2 = (world_rank + 1 == world_size) ? height : world_rank * num_per_height + num_per_height,
        .x1 = world_rank * num_per_width,
        .x2 = (world_rank + 1 == world_size) ? width : world_rank * num_per_width + num_per_width,
    };

    printf("rank:%d y1:%d y2:%d x1:%d x2:%d\n", world_rank, pos.y1, pos.y2, pos.x1, pos.x2);

    MPI_Finalize();
}
