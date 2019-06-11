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
int iterations = 10000000;
int world_size, world_rank;
int height = 40;
int width = 160;
int keys = 0;
int key = 1;
int zoom_i = 0;
int zoom_o = 0;
int rgt = 0;
int lft = 0;
int down = 0;
int up = 0;
float delta[2];

volatile sig_atomic_t interrupted = 0;

void interrupt_signal(int);
void zoom_out(int);
void zoom_in(int);
void mv_rgt(int);
void mv_lft(int);
void mv_up(int);
void mv_down(int);
unsigned int mandelbrot(float x, float y);
float **alloc_2d(int rows, int cols);
int process();

int main(int argc, char *argv[])
{
    signal(SIGINT, interrupt_signal);
    signal(SIGVTALRM, zoom_out);
    signal(SIGUSR1, zoom_in);
    signal(SIGUSR2, mv_rgt);
    signal(SIGINFO, mv_lft);
    signal(SIGWINCH, mv_up);
    signal(SIGPROF, mv_down);
    putenv("LINES=40");
    putenv("COLUMNS=160");

    if (argv[1] != NULL) {
        sscanf(argv[1], "%d", &iterations);
    }

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int ch;
    if (world_rank == 0) {
        initscr();
        nodelay(stdscr, TRUE);
        curs_set(FALSE);
    }

    while (!interrupted) {
        if (keys != key) {
            process();
            keys++;
        }
        sleep(1);
        if (world_rank == 0) {
            ch = getch();
            if (ch == 105) {
                ch = ERR;
                ungetch(ch);
                clear();
                refresh();
                system("killall -SIGUSR1 mandelbrot");
            }
            if (ch == 106) {
                ch = ERR;
                ungetch(ch);
                clear();
                refresh();
                system("killall -SIGPROF mandelbrot");
            }
            if (ch == 107) {
                ch = ERR;
                ungetch(ch);
                clear();
                refresh();
                system("killall -SIGWINCH mandelbrot");
            }
            if (ch == 104) {
                ch = ERR;
                ungetch(ch);
                clear();
                refresh();
                system("killall -SIGUSR2 mandelbrot");
            }
            if (ch == 108) {
                ch = ERR;
                ungetch(ch);
                clear();
                refresh();
                system("killall -SIGINFO mandelbrot");
            }
            if (ch == 111) {
                ch = ERR;
                ungetch(ch);
                clear();
                refresh();
                system("killall -SIGVTALRM mandelbrot");
            }
        }
    }

    if (world_rank == 0) {
        curs_set(1);
        endwin();
    }

    MPI_Finalize();
}

int process()
{
    MPI_Status stat;
    MPI_Request request;

    delta[0] = (float)(view_x[1] - view_x[0]) / width;
    delta[1] = (float)(view_y[1] - view_y[0]) / height;

    int columns, rows;
    swich(world_size){
	    case 2: 
			columns = 20;
	        rows = 160;
	    	break;
	    case 4: 
	        columns = 20;
	        rows = 80;
	        break;
	    case 8:
	        columns = 10;
	        rows = 80;
	        break;
	    case 16:
	        columns = 10;
	        rows = 40;
	        break;
	    case 32:
	        columns = 5;
	        rows = 40;
	        break;
	    case 64:
	        columns = 5;
	        rows = 20;
	        break;
	    case 128:
	        columns = 5;
	        rows = 10;
	        break;
	    case 256:
	        columns = 5;
	        rows = 5;
	        break;
	    default:
			//do nothing
			break;
    }


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

    float **sub_arr = alloc_2d(total_y * total_x, 4);

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
                sub_arr[index][3] = 0;
            } else {
                sub_arr[index][2] = 1;
                sub_arr[index][3] = (float) its;
            }

            index++;
        }
    }

    MPI_Isend(&(sub_arr[0][0]), total_y * total_x * 4, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, &request);

    if (world_rank == 0) {
        float **arr_colors = alloc_2d(height, width);
        for (int j = 0; j < height; j++) {
            for (int i = 0; i < width; i++) {
                arr_colors[j][i] = 0;
            }
        }
        float **arr = alloc_2d(height, width);
        for (int j = 0; j < height; j++) {
            for (int i = 0; i < width; i++) {
                arr[j][i] = 1000;
            }
        }
        float **sub_arr = alloc_2d(total_y * total_x, 4);
        start_color();
        int colors = COLOR_PAIRS - 1;

        for(short i=1; i < colors; i++){
            init_pair(i, i, COLOR_BLACK);
        }

        for (int k = 0; k < world_size; k++) {
            MPI_Recv(&(sub_arr[0][0]), total_y * total_x * 4, MPI_FLOAT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &stat);

            for (int i = 0; i < total_y * total_x; i++) {
                int y = (int) sub_arr[i][0];
                int x = (int) sub_arr[i][1];
                arr[y][x] = sub_arr[i][2];
                arr_colors[y][x] = sub_arr[i][3];
            }

            for (int j = 0; j < height; j++) {
                for (int i = 0; i < width; i++) {
                    if ((int) arr[j][i] == 0) {
                        attron(COLOR_PAIR(0));
                        mvaddch(j, i, '*');
                        attroff(COLOR_PAIR(0));
                    } else if ((int) arr[j][i] == 1000) {
                        attron(COLOR_PAIR(0));
                        mvaddch(j, i, ' ');
                        attroff(COLOR_PAIR(0));
                    } else {
                        int colcode = ((int) arr_colors[j][i] > colors) ? COLOR_PAIRS-1 : (int) arr_colors[j][i];
                        attron(COLOR_PAIR(colcode));
                        mvaddch(j, i, '.');
                        attroff(COLOR_PAIR(colcode));
                    }
                    refresh();
                }
            }
        }
    }

    return 0;
}

float **alloc_2d(int rows, int cols)
{
    float *data = (float *)malloc(rows*cols*sizeof(float));
    float **array= (float **)malloc(rows*sizeof(float*));
    for (int i=0; i<rows; i++)
        array[i] = &(data[cols*i]);

    return array;
}

unsigned int mandelbrot(float x, float y)
{
    float complex c = x + y*I;
    float complex z = 0;
    for (unsigned int i=0; i < iterations; i++)
    {
        if (cabsf(z) > 2) { return i; }
        z = z*z + c;
    }
    return 0;
}

//Set a flag saying that we were interrupted.
void interrupt_signal(int param){
    interrupted = 1;
}

void zoom_out(int param)
{
    zoom_o++;

    float delta_y = 0;
    float delta_x = 0;

    for (int j = 0; j < down; j++) {
        delta_y += (view_y[1] - view_y[0]) * 0.22;
    }
    for (unsigned int i = 0; i < 2; i++) {
        view_y[i] += delta_y;
    }

    for (int j = 0; j < up; j++) {
        delta_y += (view_y[1] - view_y[0]) * 0.22;
    }
    for (unsigned int i = 0; i < 2; i++) {
        view_y[i] -= delta_y;
    }

    // RGT ------------

    for (int j = 0; j < rgt; j++) {
        delta_x += (view_x[1] - view_x[0]) * 0.22;
    }
    for (unsigned int i = 0; i < 2; i++) {
        view_x[i] -= delta_x;
    }

    // LFT -----------

    for (int j = 0; j < lft; j++) {
        delta_x += (view_x[1] - view_x[0]) * 0.22;
    }
    for (unsigned int i = 0; i < 2; i++) {
        view_x[i] += delta_x;
    }



    for (int j = 0; j < zoom_o; j++) {
        for (unsigned int i = 0; i < 2; i++) {
            view_y[i] *= 1.1;
            view_y[i] -= delta_y;
            view_x[i] *= 1.1;
            view_x[i] -= delta_x;
        }
    }
    key++;
}
void zoom_in(int param)
{
    zoom_i++;

    float delta_y = 0;
    float delta_x = 0;

    for (int j = 0; j < down; j++) {
        delta_y += (view_y[1] - view_y[0]) * 0.22;
    }
    for (unsigned int i = 0; i < 2; i++) {
        view_y[i] += delta_y;
    }

    for (int j = 0; j < up; j++) {
        delta_y += (view_y[1] - view_y[0]) * 0.22;
    }
    for (unsigned int i = 0; i < 2; i++) {
        view_y[i] -= delta_y;
    }

    for (int j = 0; j < rgt; j++) {
        delta_x += (view_x[1] - view_x[0]) * 0.22;
    }
    for (unsigned int i = 0; i < 2; i++) {
        view_x[i] -= delta_x;
    }

    for (int j = 0; j < lft; j++) {
        delta_x += (view_x[1] - view_x[0]) * 0.22;
    }
    for (unsigned int i = 0; i < 2; i++) {
        view_x[i] += delta_x;
    }



    for (int j = 0; j < zoom_i; j++) {
        for (unsigned int i = 0; i < 2; i++) {
            view_y[i] *= 0.90;
            view_y[i] -= delta_y;
            view_x[i] *= 0.90;
            view_x[i] -= delta_x;
        }
    }
    key++;
}

void mv_lft(int param)
{
    lft++;
    if (rgt > 1) {
        rgt--;
    } else {
        rgt = 0;
    }

    float delta_x = 0;

    for (int j = 0; j < lft; j++) {
        delta_x += (view_x[1] - view_x[0]) * 0.22;
    }
    for (unsigned int i = 0; i < 2; i++) {
        view_x[i] += delta_x;
    }
    key++;
}
void mv_rgt(int param)
{
    rgt++;
    if (lft > 1) {
        lft--;
    } else {
        lft = 0;
    }
    float delta_x = 0;

    for (int j = 0; j < rgt; j++) {
        delta_x += (view_x[1] - view_x[0]) * 0.22;
    }
    for (unsigned int i = 0; i < 2; i++) {
        view_x[i] -= delta_x;
    }
    key++;
}

void mv_up(int param)
{
    up++;
    if (down > 1) {
        down--;
    } else {
        down = 0;
    }
    float delta_y = 0;

    for (int j = 0; j < up; j++) {
        delta_y += (view_y[1] - view_y[0]) * 0.22;
    }
    for (unsigned int i = 0; i < 2; i++) {
        view_y[i] -= delta_y;
    }
    key++;
}

void mv_down(int param)
{
    down++;
    if (up > 1) {
        up--;
    } else {
        up = 0;
    }
    float delta_y = 0;

    for (int j = 0; j < down; j++) {
        delta_y += (view_y[1] - view_y[0]) * 0.22;
    }
    for (unsigned int i = 0; i < 2; i++) {
        view_y[i] += delta_y;
    }
    key++;
}
