#include <stdlib.h>
#include <stdio.h>
#include <complex.h>
#include <unistd.h>
#include <curses.h>
#include <signal.h>
#include <mpi.h>

unsigned int mandelbrot(float x, float y);
void interrupt_signal(int);

//Signal flag for interrupts.
volatile sig_atomic_t interrupted = 0;

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
    signal(SIGINT, interrupt_signal);

    //Initialise ncurses, hide the cursor and get width/height.
    initscr();
    curs_set(0);
    getmaxyx(stdscr, height, width);
    //Initialise colour pairs if we can.
    start_color();
    for(short i=1; i < COLORS; i++){
        init_pair(i, i, COLOR_BLACK);
    }

    //printf("h:%d, width:%d", height, width);

    float ***xy = alloc_data(height,width,3);

    move(0,0);
    //Difference from view_x and view_y start points in real coords.
    float delta[] = {
        (float)(view_x[1] - view_x[0]) / width, 
        (float)(view_y[1] - view_y[0]) / height };

    for(int j=0; j<height && !interrupted; j++){
        for(int i=0; i<width && !interrupted; i++){
            xy[j][i][0] = view_x[0] + delta[0] * i;
            xy[j][i][1] = view_y[0] + delta[1] * j;
        }
    }

    //if (world_rank == 0) {
    MPI_Send(&xy, 1, MPI_FLOAT, 1, 0, MPI_COMM_WORLD);
    //}

    for(int j=0; j<height && !interrupted; j++){
        for(int i=0; i<width && !interrupted; i++){
            unsigned int its = mandelbrot(xy[j][i][0],xy[j][i][1]);
            if(its == 0){
                attron(COLOR_PAIR(0));
                mvaddch(j,i,'*');
                attroff(COLOR_PAIR(0));
            }else{
                unsigned int colcode = (its > COLORS) ? COLOR_PAIRS-1 : its;
                attron(COLOR_PAIR(colcode));
                mvaddch(j,i,'.');
                attroff(COLOR_PAIR(colcode));
            }
        }
    }

    refresh();

    while(!interrupted){
    }

    curs_set(1);
    endwin();
    return 0;
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

//Set a flag saying that we were interrupted.
void interrupt_signal(int param){
    interrupted = 1;
}
