#!/bin/sh
#SBATCH --job-name=para_test299
#SBATCH --nodes=1
#SBATCH --time=00:05:00
#SBATCH --partition=Project
#SBATCH --ntasks=4

mpirun -np 4 ./psort 100000 ./test_data/10000a.in




