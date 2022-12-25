#!/bin/bash
#SBATCH --job-name=your_job_name # Job name
#SBATCH --nodes=1                    # Run all processes on a single node	
#SBATCH --ntasks=1                   # number of processes = 1 
#SBATCH --cpus-per-task=20      # Number of CPU cores allocated to each process
#SBATCH --partition=Project            # Partition name: Project or Debug (Debug is default)

cd /nfsmnt/120090266/CSC4005_2022Fall_Demo/project3_template/
./pthread 5000 100 1
# ./pthread 5000 100 4
# ./pthread 1000 100 20
# ./pthread 5000 100 20
# ./pthread 5000 100 40
# ./pthread 1000 100 120
# ./pthread 1000 100 200

./pthread 200 100 1
# ./pthread 200 100 4
# # ./pthread 1000 100 20
# ./pthread 200 100 20
# ./pthread 200 100 40

./pthread 10000 100 1
# ./pthread 10000 100 4
# # ./pthread 1000 100 20
# ./pthread 10000 100 20
# ./pthread 10000 100 40