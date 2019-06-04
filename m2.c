#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <complex.h>
#include <unistd.h>
#include <math.h>

float **alloc_2d_int(int rows, int cols);

int main(int argc, char *argv[])
{
    int i, world_size, world_rank;
    int l = 20;
    int m = 20;
    float **arr = alloc_2d_int(l, m);

    int count = 0;
    for (int i = 0; i < l; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = count;
            count += 1.0f;
        }
    }

    MPI_Status stat;
    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int y2 = (world_rank * 5);
    int x2 = (world_rank * 5);
    int y1 = y2 - 5;
    int x1 = x2 - 5;

    if (world_rank == 0) {
        for (i = 1; i < world_size; i++) {
            float **other_arr = alloc_2d_int(l, m);

            MPI_Recv(&(other_arr[0][0]), l*m, MPI_FLOAT, i, 0, MPI_COMM_WORLD, &stat);

            for (int j = y1; j < y2; j++) {
                for (int i = x1; i < x2; i++) {
                    printf("%d ", (int) other_arr[i][j]);
                    arr[j][i] = other_arr[j][i];
                }
            }

            printf("rank recved:%d \n", i);
        }
        for (int i = 0; i < l; i++) {
            for (int j = 0; j < m; j++) {
                printf("%d ", (int) arr[i][j]);
            }
            printf("\n");
        }

    } else {
        printf("rank: %d\n", world_rank);

        float **other_arr = alloc_2d_int(l, m);

        float count = y1+x1;
        for (int i = y1; i < y2; i++) {
            for (int j = x1; j < x2; j++) {
                other_arr[i][j] = count; 
                count += 1.0f;
            }
        }

        MPI_Send(&(other_arr[0][0]), l*m, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return 0;
}

float **alloc_2d_int(int rows, int cols)
{
    float *data = (float *)malloc(rows*cols*sizeof(float));
    float **array= (float **)malloc(rows*sizeof(float*));
    for (int i=0; i<rows; i++)
        array[i] = &(data[cols*i]);

    return array;
}
