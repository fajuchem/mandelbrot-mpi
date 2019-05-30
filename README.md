mpicc -omandelbrot -Wall -std=c99 mandelbrot.c -lm -lrt   -lncursesw -ltinfo
mpirun -m 2 ./mandebrot
