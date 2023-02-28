#!/bin/bash
#SBATCH --job-name=a3_hybrid
#SBATCH --account=eel6763
#SBATCH --nodes=1  #(Single node)
#SBATCH --ntasks=8 #(single process)
#SBATCH --cpus-per-task=4 #(limit is 32)
#SBATCH --mem-per-cpu=600mb
#SBATCH -t 00:00:20
#SBATCH -o out.txt
#SBATCH -e err.txt
export OMP_NUM_THREADS=8
#(NRA, NCA_RB, NCB)
srun ./matmult_hybrid 60 12 10

