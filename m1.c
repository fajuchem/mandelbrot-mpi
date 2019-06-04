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

void free_data(float ***data, size_t xlen, size_t ylen);
float ***alloc_data(size_t xlen, size_t ylen, size_t zlen);
unsigned int mandelbrot(float x, float y);

int main(int argc, char *argv[])
{
    int i, world_size, world_rank;
    int l = 45;
    int m = 178;
    int n = 3;
    float ***arr = alloc_data(l, m, n);
    float ***other_arr = alloc_data(l, m, n);

    MPI_Status stat;
    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    float delta[] = {
        (float)(view_x[1] - view_x[0]) / m,
        (float)(view_y[1] - view_y[0]) / l };

    for(int j=0; j<l; j++){
        for(int i=0; i<m; i++){
            // x
            other_arr[j][i][0] = view_x[0] + delta[0] * i;
            arr[j][i][0] = view_x[0] + delta[0] * i;
            // y
            other_arr[j][i][1] = view_y[0] + delta[1] * j;
            arr[j][i][1] = view_y[0] + delta[1] * j;
        }
    }

    //if (world_rank == 0) {
    //    for (int i = 0; i < l; i++) {
    //        for (int j = 0; j < m; j++) {
    //            //printf("i:%d j:%d\n", i, j);
    //            unsigned int its = mandelbrot(other_arr[i][j][0], other_arr[i][j][1]);
    //            if (its == 0) {
    //                other_arr[i][j][2] = 0;
    //                printf("*");
    //            } else {
    //                other_arr[i][j][2] = 1;
    //                printf(".");
    //            }
    //        }
    //        printf("\n");
    //    }
    //    printf("\n");
    //}
    //MPI_Finalize();


    if ( world_rank == 0 ) {
        for (i = 1; i < world_size; i++)
        {
            MPI_Recv(&(other_arr[0][0][0]), l*m*n, MPI_FLOAT, i, 0, MPI_COMM_WORLD, &stat);

            printf("rank recved:%d \n", i);

            for (int a = 0; a < l; a++) {
                arr[i][a][0] = other_arr[i][a][0];
                arr[i][a][1] = other_arr[i][a][1];
                arr[i][a][2] = other_arr[i][a][2];
            }
        }

        for (int i = 1; i < l; i++) {
            for (int j = 0; j < m; j++) {
                if (arr[i][j][2] == 0) {
                    printf("*");
                } else {
                    printf("-");
                }
            }
            printf("\n");
        }

    } else {
        printf("rank: %d\n", world_rank);

        for (int i = 0; i < m; i++) {
            unsigned int its = mandelbrot(other_arr[world_rank][i][0], other_arr[world_rank][i][1]);
            if (its == 0) {
                other_arr[world_rank][i][2] = 0;
            } else {
                other_arr[world_rank][i][2] = 1;
            }
            //printf("i:%d r:%d n:%f\n", i, world_rank, other_arr[world_rank][i][2]);
        }


        MPI_Send(&(other_arr[0][0][0]), l*m*n, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
    }

    //free_data(other_arr, m, l);
    //free_data(arr, m, l);

    MPI_Finalize();

    return 0;
}

void free_data(float ***data, size_t xlen, size_t ylen)
{
    size_t i, j;

    for (i=0; i < xlen; ++i) {
        if (data[i] != NULL) {
            for (j=0; j < ylen; ++j)
                free(data[i][j]);
            free(data[i]);
        }
    }
    free(data);
}

float ***alloc_data(size_t xlen, size_t ylen, size_t zlen)
{
    float ***p;
    size_t i, j;

    if ((p = malloc(xlen * sizeof *p)) == NULL) {
        perror("malloc 1");
        return NULL;
    }

    for (i=0; i < xlen; ++i)
        p[i] = NULL;

    for (i=0; i < xlen; ++i)
        if ((p[i] = malloc(ylen * sizeof *p[i])) == NULL) {
            perror("malloc 2");
            free_data(p, xlen, ylen);
            return NULL;
        }

    for (i=0; i < xlen; ++i)
        for (j=0; j < ylen; ++j)
            p[i][j] = NULL;

    for (i=0; i < xlen; ++i)
        for (j=0; j < ylen; ++j)
            if ((p[i][j] = malloc(zlen * sizeof *p[i][j])) == NULL) {
                perror("malloc 3");
                free_data(p, xlen, ylen);
                return NULL;
            }

    return p;
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
