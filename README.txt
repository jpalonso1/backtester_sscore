Project: GPU based backtesting framework
Author: Juan Pablo Alonso

This program separates noise from signal given a very specific set of conditions.

REQUIREMENTS: 
-Linux
-Nvidia Cuda framework
-for gpu backend: Nvidia card compatible with CUDA 5.0

Compile with 
$make 
to run the gpu based version, or
$make -f makeomp
to run using openMP backend (slower)