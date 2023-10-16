#!/bin/bash
#SBATCH --job-name=your_job_name # Job name
#SBATCH --nodes=2                    # Run all processes on a single node	
#SBATCH --ntasks=40                   # number of processes = 20
#SBATCH --cpus-per-task=2      # Number of CPU cores allocated to each process (please use 1 here, in comparison with pthread)
#SBATCH --partition=Project            # Partition name: Project or Debug (Debug is default)

cd /nfsmnt/120090266/CSC4005_2022Fall_Demo/project4_template/
# mpirun -np 4 ./mpi 200 
# mpirun -np 4 ./mpi 500 
# mpirun -np 4 ./mpi 1000 
# mpirun -np 4 ./mpi 2000 

# mpirun -np 20 ./mpi 200 
# mpirun -np 20 ./mpi 400 
# mpirun -np 20 ./mpi 1000 
# mpirun -np 20 ./mpi 2000 


# mpirun -np 40 ./mpi 200 
# mpirun -np 40 ./mpi 400 
# mpirun -np 40 ./mpi 1000 
mpirun -np 39 ./mpi 2000 
# mpirun -np 20 ./mpi 1000 
# mpirun -np 40 ./mpi 1000 