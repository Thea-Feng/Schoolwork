#!/bin/bash
#SBATCH --job-name=your_job_name # Job name
#SBATCH --nodes=1                    # Run all processes on a single node	
#SBATCH --ntasks=1                   # number of processes = 1 
#SBATCH --cpus-per-task=20      # Number of CPU cores allocated to each process
#SBATCH --partition=Project            # Partition name: Project or Debug (Debug is default)

cd /nfsmnt/120090266/CSC4005_2022Fall_Demo/project3_template/
./openmp 200 100 1
# ./openmp 200 100 4
# ./openmp 200 100 20
# ./openmp 200 100 40

./openmp 5000 100 1
# ./openmp 5000 100 4
# ./openmp 5000 100 20
# ./openmp 5000 100 40

./openmp 10000 100 1
# ./openmp 10000 100 4
# ./openmp 10000 100 20
# ./openmp 10000 100 40
# ./openmp 1000 100 80
# ./openmp 1000 100 120
# ./openmp 1000 100 200