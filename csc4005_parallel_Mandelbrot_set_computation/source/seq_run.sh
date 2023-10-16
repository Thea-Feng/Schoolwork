#!/bin/bash
#SBATCH --job-name=seq # Job name
#SBATCH --nodes=1                    # Run all processes on a single node	
#SBATCH --ntasks=1                   # number of processes = 1 
#SBATCH --cpus-per-task=20      # Number of CPU cores allocated to each process
#SBATCH --partition=Project            # Partition name: Project or Debug (Debug is default)

cd /nfsmnt/120090266/CSC4005_2022Fall_Demo/project2_template-2/project2_template/
./seq 50 50 100 
./seq 100 100 100 
./seq 200 200 100 

./seq 600 600 100 
./seq 1000 1000 100 
./seq 2000 2000 100 


./seq 4000 8000 100 
./seq 8000 8000 100 
./seq 10000 10000 100 

# ./pthread 1000 1000 100 80
# ./pthread 1000 1000 100 120
# ./pthread 1000 1000 100 200