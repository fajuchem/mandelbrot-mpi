#!/bin/bash

make && mpirun -np $1 ./mandelbrot $2
