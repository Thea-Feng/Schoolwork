#!/bin/bash
#SBATCH --job-name=pthread # Job name
#SBATCH --nodes=1                    # Run all processes on a single node	
#SBATCH --ntasks=1                   # number of processes = 1 
#SBATCH --cpus-per-task=40      # Number of CPU cores allocated to each process
#SBATCH --partition=Project            # Partition name: Project or Debug (Debug is default)

cd /nfsmnt/120090266/CSC4005_2022Fall_Demo/project2_template-2/project2_template/
./pthread 50 50 100 32
./pthread 100 100 100 32
./pthread 200 200 100 32

./pthread 600 600 100 32
./pthread 1000 1000 100 32
./pthread 2000 2000 100 32


./pthread 4000 8000 100 32
./pthread 8000 8000 100 32
./pthread 10000 10000 100 32