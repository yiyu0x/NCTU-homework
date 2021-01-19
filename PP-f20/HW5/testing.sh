#!/bin/bash

for view in 1 2;do
    for iter in 1000 10000 100000;do
        echo "./mandelbrot --iter $iter -v $view";
        ./mandelbrot --iter $iter -v $view | tail -2 | sed 's/^\t*//g';
    done
done