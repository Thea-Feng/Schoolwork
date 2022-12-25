#!/bin/bash
#SBATCH --job-name=your_job_name # Job name
#SBATCH --nodes=1                    # Run all processes on a single node	
#SBATCH --ntasks=1                   # number of processes = 1 
#SBATCH --cpus-per-task=20      # Number of CPU cores allocated to each process
#SBATCH --partition=Project            # Partition name: Project or Debug (Debug is default)

cd /nfsmnt/120090266/CSC4005_2022Fall_Demo/project4_template/
# ./pthread 200 20
# ./pthread 500 20
# ./pthread 1000 20
# ./pthread 2000 20
# ./pthread 1000 
./pthread 1000 4
./pthread 1000 20
./pthread 1000 40

# ./pthread 200 20
# ./pthread 500 20
# ./pthread 1000 20
# ./pthread 2000 20
# ./pthread 1000 100 120
# ./pthread 1000 100 200