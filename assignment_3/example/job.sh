#!/bin/bash
#SBATCH --job-name=a3_mpi_example
#SBATCH --account=eel6763
#SBATCH --nodes=2  #(num nodes)
#SBATCH --ntasks=2 #(num processes)
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=8 #(limit is 32)
#SBATCH --mem-per-cpu=600mb
#SBATCH -t 00:05:00
#SBATCH -o out.txt
#SBATCH -e err.txt
#(NRA, NCA_RB, NCB)
srun ./matmult 60 12 10

