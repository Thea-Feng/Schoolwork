#!/bin/bash
#SBATCH --job-name=mpi # Job name
#SBATCH --nodes=1                    # Run all processes on a single node	
#SBATCH --ntasks=40                   # number of processes = 20
#SBATCH --cpus-per-task=1      # Number of CPU cores allocated to each process (please use 1 here, in comparison with pthread)
#SBATCH --partition=Project            # Partition name: Project or Debug (Debug is default)
cd /nfsmnt/120090266/CSC4005_2022Fall_Demo/project2_template-2/project2_template/

mpirun -np 32 ./mpi 50 50 100 
mpirun -np 32 ./mpi 100 100 100 
mpirun -np 32 ./mpi 200 200 100 

mpirun -np 32 ./mpi 600 600 100 
mpirun -np 32 ./mpi 1000 1000 100 
mpirun -np 32 ./mpi 2000 2000 100 


mpirun -np 32 ./mpi 4000 8000 100 
mpirun -np 32 ./mpi 8000 8000 100 
mpirun -np 32 ./mpi 10000 10000 100 