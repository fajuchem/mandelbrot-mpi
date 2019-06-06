#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <complex.h>
#include <unistd.h>
#include <math.h>
#include <curses.h>

typedef struct Position {
    int y1;
    int y2;
    int x1;
    int x2;
} Position;

float view_x[] = {-2, 1};
float view_y[] = {-1, 1};
int iterations = 1000000;

unsigned int mandelbrot(float x, float y);
float **alloc_2d(int rows, int cols);

/**
 * - calcular y2, y1, x2, x1 dependendo do numero processo
 * - ncurses fica com a tela de no maximo 80x24
 * - processos recebem em ordem aleatoria mas são printados na tela em ordem
 * - alocar dinamicamente buffer do recv quando um dos processos fica com um bloco maior
 */
int main(int argc, char *argv[])
{
    int world_size, world_rank;
    int height = 24;
    int width = 80;

    float delta[] = {
        (float)(view_x[1] - view_x[0]) / width,
        (float)(view_y[1] - view_y[0]) / height
    };

    MPI_Status stat;
    MPI_Request request;
    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    // @TODO ver isso aqui....
    int columns = height / ceil(sqrt(world_size));
    int rows = width / ceil(sqrt(world_size));

    columns = 4;
    rows = 10;

    //printf("columns:%d, rows:%d\n", columns, rows);

    //printf("height:%d, width:%d, column:%d, rows:%d\n", height, width, columns, rows);
    //return 0;

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


    float **sub_arr = alloc_2d(total_y * total_x, 3);

    int index = 0;
    for (int j = pos.y1; j < pos.y2; j++) {
        for (int i = pos.x1; i < pos.x2; i++) {
            float x = view_x[0] + delta[0] * i;
            float y = view_y[0] + delta[1] * j;
            // height
            sub_arr[index][0] = (float) j;
            // width
            sub_arr[index][1] = (float) i;

            unsigned int its = mandelbrot(x, y);
            if (its == 0) {
                sub_arr[index][2] = 0;
            } else {
                sub_arr[index][2] = 1;
            }

            index++;
        }
    }

    MPI_Isend(&(sub_arr[0][0]), total_y * total_x * 3, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, &request);

    free(sub_arr);
    free(sub_arr[0]);

    if (world_rank == 0) {
        float **arr = alloc_2d(height, width);
        float **sub_arr = alloc_2d(total_y * total_x, 3);
        //initscr();
        //curs_set(0);

        for (int k = 0; k < world_size; k++) {

            //printf("\n\ntotal_y:%d, total_x:%d\n\n", total_y, total_x);

            MPI_Recv(&(sub_arr[0][0]), total_y * total_x * 3, MPI_FLOAT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &stat);

            //printf("world_rank:%d\n", stat.MPI_SOURCE);

            for (int i = 0; i < total_y * total_x; i++) {
                int y = (int) sub_arr[i][0];
                int x = (int) sub_arr[i][1];
                //printf("x:%d, y:%d, v:%f\n", x, y, sub_arr[i][2]);
                arr[y][x] = sub_arr[i][2];
                //if ((int) sub_arr[i][2] == 0) {
                //    mvaddch(y, x, '*');
                //} else {
                //    mvaddch(y, x, ' ');
                //}
            }

            //for (int i = 0; i < total_y * total_x; i++) {
            //    printf("x:%f, y:%f, v:%f\n", sub_arr[i][1], sub_arr[i][0], sub_arr[i][2]);
            //}
            printf("world_rank:%d\n", stat.MPI_SOURCE);

            for (int j = 0; j < height; j++) {
                for (int i = 0; i < width; i++) {
                    if ((int) arr[j][i] == 0) {
                        printf("*");
                    } else {
                        printf(".");
                    }
                }
                printf("\n");
            }

            //for (int j = 0; j < height; j++) {
            //    for (int i = 0; i < width; i++) {
            //        if ((int) arr[j][i] == 0) {
            //            mvaddch(j, i, '*');
            //        } else {
            //            mvaddch(j, i, ' ');
            //        }
            //    }
            //}
            //refresh();
            //sleep(1);

        }
        //curs_set(1);
        //endwin();
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
