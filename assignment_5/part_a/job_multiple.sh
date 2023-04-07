#!/bin/bash
#SBATCH --mail-type=FAIL
#SBATCH --mail-user=stefenlagos@ufl.edu
#SBATCH --account=eel6763
#SBATCH --qos=eel6763
#SBATCH --nodes=8
#SBATCH --ntasks=8
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=500mb
#SBATCH -t 00:05:00
#SBATCH -o outfile
#SBATCH -e errfile
export GMON_OUT_PREFIX=gmon.out-
srun --mpi=pmix_v2 ./cmtbonebe 100 5 4 4 4 2 2 2