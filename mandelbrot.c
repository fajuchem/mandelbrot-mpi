#include <stdlib.h>
#include <stdio.h>
#include <complex.h>
#include <unistd.h>
#include <curses.h>
#include <signal.h>

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

int main(int argc, char **argv){
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

    move(0,0);
    //Difference from view_x and view_y start points in real coords.
    float delta[] = {
        (float)(view_x[1] - view_x[0]) / width, 
        (float)(view_y[1] - view_y[0]) / height };

    for(int j=0; j<height && !interrupted; j++){
        for(int i=0; i<width && !interrupted; i++){
            float x = view_x[0] + delta[0] * i;
            float y = view_y[0] + delta[1] * j;

            unsigned int its = mandelbrot(x,y);
            if(its == 0){
                attron(COLOR_PAIR(0));
                addch('*');
                attroff(COLOR_PAIR(0));
            }else{
                unsigned int colcode = (its > COLORS) ? COLOR_PAIRS-1 : its;
                attron(COLOR_PAIR(colcode));
                addch('.');
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
