#!/bin/bash
#SBATCH --job-name=a3
#SBATCH --account=eel6763
#SBATCH --nodes=1  #(Single node)
#SBATCH --ntasks=1 #(single process)
#SBATCH --cpus-per-task=8 #(limit is 32)
#SBATCH --mem-per-cpu=600mb
#SBATCH -t 00:05:00
#SBATCH -o out.txt
#SBATCH -e err.txt
export OMP_NUM_THREADS=4
#(NRA, NCA_RB, NCB)
./matmult 60 12 10

