#!/bin/bash
#SBATCH --job-name=your_job_name # Job name
#SBATCH --nodes=2                    # Run all processes on a single node	
#SBATCH --ntasks=40                   # number of processes = 20
#SBATCH --cpus-per-task=2      # Number of CPU cores allocated to each process (please use 1 here, in comparison with pthread)
#SBATCH --partition=Project            # Partition name: Project or Debug (Debug is default)

cd /nfsmnt/120090266/CSC4005_2022Fall_Demo/project4_template/
# mpirun -np 1 ./mpi_openmp 200 1
# mpirun -np 1 ./mpi_openmp 400 1
# mpirun -np 1 ./mpi_openmp 1000 1
# mpirun -np 1 ./mpi_openmp 2000 1

# mpirun -np 2 ./mpi_openmp 200 2
# mpirun -np 2 ./mpi_openmp 400 2
# mpirun -np 2 ./mpi_openmp 1000 2
# mpirun -np 2 ./mpi_openmp 2000 2

# mpirun -np 5 ./mpi_openmp 200 4
# mpirun -np 5 ./mpi_openmp 400 4
# mpirun -np 5 ./mpi_openmp 1000 4
# mpirun -np 5 ./mpi_openmp 2000 4

# mpirun -np 8 ./mpi_openmp 200 5
# mpirun -np 8 ./mpi_openmp 400 5
# mpirun -np 8 ./mpi_openmp 1000 5
# mpirun -np 8 ./mpi_openmp 2000 5

mpirun -np 21 ./mpi_openmp 2000 2
