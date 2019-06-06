#!/bin/bash

make && mpirun -np 48 ./mandelbrot
