#include <stdlib.h>
#include <stdio.h>
#include <complex.h>
#include <unistd.h>
#include <curses.h>
#include <mpi.h>

unsigned int mandelbrot(float x, float y);

//There's not much point in having many iterations
unsigned int iterations = 256;
unsigned int width, height;

//The viewport in real coordinates (i.e. not in lines/characters).
float view_x[] = {-2, 1};
float view_y[] = {-1, 1};

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

int main(int argc, char **argv){
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);
    // Find out rank, size
    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    //Set a flag upon interrupt so that we can clean up properly.
    //signal(SIGINT, interrupt_signal);

    //Initialise ncurses, hide the cursor and get width/height.
    //initscr();
    //curs_set(0);
    //getmaxyx(stdscr, height, width);
    //Initialise colour pairs if we can.
    
    int width = 236;
    int height = 55;


    //start_color();
    //for(short i=1; i < COLORS; i++){
    //    init_pair(i, i, COLOR_BLACK);
    //}

    //printf("h:%d, width:%d", height, width);

    float ***xy = alloc_data(height,width,3);

    //move(0,0);
    //Difference from view_x and view_y start points in real coords.
    float delta[] = {
        (float)(view_x[1] - view_x[0]) / width, 
        (float)(view_y[1] - view_y[0]) / height };

    for(int j=0; j<height; j++){
        for(int i=0; i<width; i++){
            xy[j][i][0] = view_x[0] + delta[0] * i;
            xy[j][i][1] = view_y[0] + delta[1] * j;
        }
    }

    if (world_rank == 0) {
        MPI_Send(&xy[0], width*3, MPI_FLOAT, 1, 0, MPI_COMM_WORLD);
        printf("rank0\n");
    } else {
        MPI_Recv(&xy[0], width*3, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        printf("%f\n", xy[0][0][0]);

        for(int i=0; i<width; i++){
            unsigned int its = mandelbrot(xy[0][i][0],xy[0][i][1]);
            if(its == 0){
                xy[0][i][2] = 0;
            }else{
                xy[0][i][2] = 1;
            }
        }

        printf("rank1\n");
        MPI_Send(&xy[0], width*3, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
        printf("rank1\n");
    }

    printf("oi\n");

    if (world_rank == 0) {
        printf("desenho\n");
        MPI_Recv(&xy[0], width*3, MPI_FLOAT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("depois desenho\n");
        for(int i=0; i<width; i++){
            if (xy[0][i][2] == 0) {
                printf("*");
            } else {
                printf(" ");
            }
        }
        printf("\n");
    }

    MPI_Finalize();
}

//Return the number of iterations taken for a coordinate to blow up -- that is,
//to increase above 2, at which point it is guaranteed to continue increasing.
//Returns 0 if the point never blows up; i.e. it is in the set.
unsigned int mandelbrot(float x, float y){
    float complex c = x + y*I;
    //Iterate. Return the number of iterations if z blows up.
    float complex z = 0;
    for(unsigned int i=0; i < iterations; i++){
        if(cabsf(z) > 2){ return i; }
        z = z*z + c;
    }
    return 0;
}
