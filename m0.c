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

float **alloc_2d(int rows, int cols);

int main(int argc, char *argv[])
{
    int world_size, world_rank;
    int height = 10;
    int width = 10;
    float **sub_arr;

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

    int total_y = pos.y2 - pos.y1;
    int total_x = pos.x2 - pos.x1;

    if (world_rank != 0) {
        sub_arr = alloc_2d(total_y * total_x, 3);

        int index = 0;
        for (int j = pos.y1; j < pos.y2; j++) {
            for (int i = pos.x1; i < pos.x2; i++) {
                // height
                sub_arr[index][0] = (float) j;
                // width
                sub_arr[index][1] = (float) i;
                // value
                sub_arr[index][2] = (float) world_rank;
                index++;
            }
        }
        //for (int i = 0; i < (total_y * total_x); i++) {
        //    printf("x:%d, y:%d, v:%f\n", (int)sub_arr[i][1], (int) sub_arr[i][0], sub_arr[i][2]);
        //}

        MPI_Send(&(sub_arr[0][0]), total_y * total_x * 3, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
    } else {
        float **arr = alloc_2d(height, width);
        for (int k = 1; k < world_size; k++) {
            sub_arr = alloc_2d(total_y * total_x, 3);

            printf("\n\ntotal_y:%d, total_x:%d\n\n", total_y, total_x);

            MPI_Recv(&(sub_arr[0][0]), total_y * total_x * 3, MPI_FLOAT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &stat);

            for (int i = 0; i < total_y * total_x; i++) {
                int y = (int) sub_arr[i][0];
                int x = (int) sub_arr[i][1];
                printf("x:%d, y:%d, v:%f\n", x, y, sub_arr[i][2]);
                arr[y][x] = sub_arr[i][2];
                printf("arr:%f\n", arr[y][x]);
            }

            //for (int i = 0; i < total_y * total_x; i++) {
            //    printf("x:%f, y:%f, v:%f\n", sub_arr[i][1], sub_arr[i][0], sub_arr[i][2]);
            //}

            for (int j = 2; j < height; j++) {
                for (int i = 2; i < width; i++) {
                    printf("%f ", arr[j][i]);
                }
                printf("\n ");
            }
        }
    }

    //printf("total_y:%d, total_x:%d \n", total_y, total_x);
    //printf("rank:%d y1:%d y2:%d x1:%d x2:%d\n", world_rank, pos.y1, pos.y2, pos.x1, pos.x2);

    MPI_Finalize();
}

float **alloc_2d(int rows, int cols)
{
    float *data = (float *)malloc(rows*cols*sizeof(float));
    float **array= (float **)malloc(rows*sizeof(float*));
    for (int i=0; i<rows; i++)
        array[i] = &(data[cols*i]);

    return array;
}
