#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <complex.h>
#include <unistd.h>
#include <math.h>

float view_x[] = {-2, 1};
float view_y[] = {-1, 1};
unsigned int iterations = 256;

int **alloc_2d_int(int rows, int cols)
unsigned int mandelbrot(float x, float y);

int main(int argc, char *argv[])
{
    int i, world_size, world_rank;
    int l = 20;
    int m = 40;
    float **arr = alloc_2d_int(l, m);

    MPI_Status stat;
    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    float delta[] = {
        (float)(view_x[1] - view_x[0]) / m,
        (float)(view_y[1] - view_y[0]) / l
    };

    if ( world_rank == 0 ) {
        for (i = 1; i < world_size; i++)
        {
            MPI_Recv(&(arr[0][0]), l*m, MPI_FLOAT, i, 0, MPI_COMM_WORLD, &stat);

            printf("rank recved:%d \n", i);
        }

        for (int i = 1; i < l; i++) {
            for (int j = 0; j < m; j++) {
                if (arr[i][j] == 0) {
                    printf("*");
                } else {
                    printf("-");
                }
            }
            printf("\n");
        }

    } else {
        printf("rank: %d\n", world_rank);

        // 5 - 1 / 20 = 5
        int y_size = (world_size - 1) / l;
        // 5 - 1 / 20 = 5
        int x_size = (world_size - 1) / m;

        // 1 * 5 = 5
        int y1 = (world_rank * y_size) - 1;

        for(int j = 0; j < l; j++) {
            for (int i = 0; i < m; i++) {
                float x = view_x[0] + delta[0] * i;
                float y = view_y[0] + delta[1] * j;
                unsigned int its = mandelbrot(x, y);
                if (its == 0) {
                    other_arr[j][i] = 0;
                } else {
                    other_arr[j][i] = 1;
                }
            }
        }

        MPI_Send(&(arr[0][0]), l*m, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return 0;
}

int **alloc_2d_int(int rows, int cols)
{
    int *data = (int *)malloc(rows*cols*sizeof(int));
    int **array= (int **)malloc(rows*sizeof(int*));
    for (int i=0; i<rows; i++)
        array[i] = &(data[cols*i]);

    return array;
}

//Return the number of iterations taken for a coordinate to blow up -- that is,
//to increase above 2, at which point it is guaranteed to continue increasing.
//Returns 0 if the point never blows up; i.e. it is in the set.
unsigned int mandelbrot(float x, float y)
{
    float complex c = x + y*I;
    //Iterate. Return the number of iterations if z blows up.
    float complex z = 0;
    for (unsigned int i=0; i < iterations; i++)
    {
        if (cabsf(z) > 2) { return i; }
        z = z*z + c;
    }
    return 0;
}
