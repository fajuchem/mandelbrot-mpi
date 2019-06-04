#!/bin/bash

make && mpirun -np 5 ./mandelbrot
