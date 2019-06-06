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

    int columns = height / ceil(sqrt(world_size));
    int rows = width / ceil(sqrt(world_size));

    int bla[world_size][4];

    int count = 0;
    for (int j = 0; j < height; j += columns) {
        for (int i = 0; i < width; i += rows) {
            bla[count][0] = j + columns;
            bla[count][1] = j;
            bla[count][2] = i + rows;
            bla[count][3] = i;
            count++;
        }
    }

    Position pos = {
        .y1 = bla[world_rank][1],
        .y2 = bla[world_rank][0],
        .x1 = bla[world_rank][3],
        .x2 = bla[world_rank][2],
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
        //    printf("i:%d --- x:%d, y:%d, v:%f\n", i, (int)sub_arr[i][1], (int) sub_arr[i][0], sub_arr[i][2]);
        //}

        MPI_Send(&(sub_arr[0][0]), total_y * total_x * 3, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
    } else {
        float **arr = alloc_2d(height, width);
        for (int k = 1; k < world_size; k++) {
            sub_arr = alloc_2d(total_y * total_x, 3);

            //printf("\n\ntotal_y:%d, total_x:%d\n\n", total_y, total_x);

            MPI_Recv(&(sub_arr[0][0]), total_y * total_x * 3, MPI_FLOAT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &stat);

            for (int i = 0; i < total_y * total_x; i++) {
                int y = (int) sub_arr[i][0];
                int x = (int) sub_arr[i][1];
                //printf("x:%d, y:%d, v:%f\n", x, y, sub_arr[i][2]);
                arr[y][x] = sub_arr[i][2];
            }

            //for (int i = 0; i < total_y * total_x; i++) {
            //    printf("x:%f, y:%f, v:%f\n", sub_arr[i][1], sub_arr[i][0], sub_arr[i][2]);
            //}

        }

        for (int j = 0; j < height; j++) {
            for (int i = 0; i < width; i++) {
                printf("%f ", arr[j][i]);
            }
            printf("\n ");
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
