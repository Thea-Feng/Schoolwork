#!/bin/bash
#SBATCH --job-name=your_job_name # Job name
#SBATCH --nodes=1                    # Run all processes on a single node	
#SBATCH --ntasks=20                   # number of processes = 20
#SBATCH --cpus-per-task=1      # Number of CPU cores allocated to each process (please use 1 here, in comparison with pthread)
#SBATCH --partition=Project            # Partition name: Project or Debug (Debug is default)

cd /nfsmnt/120090266/CSC4005_2022Fall_Demo/project3_template/
# mpirun -np 1 ./mpi_openmp 5000 100 10
# mpirun -np 4 ./mpi_openmp 5000 100 10
# mpirun -np 20 ./mpi_openmp 5000 100 10
# mpirun -np 40 ./mpi_openmp 5000 100 10

# mpirun -np 1 ./mpi_openmp 200 100 10
# mpirun -np 4 ./mpi_openmp 200 100 10
# mpirun -np 20 ./mpi_openmp 200 100 10
# mpirun -np 40 ./mpi_openmp 200 100 10

mpirun -np 1 ./mpi_openmp 10000 100 1
mpirun -np 1 ./mpi_openmp 5000 100 1
mpirun -np 1 ./mpi_openmp 1000 100 1
mpirun -np 1 ./mpi_openmp 200 100 1