#!/bin/bash
#SBATCH --job-name=mat_mult
#SBATCH --mail-type=FAIL
#SBATCH --mail-user=benjaminwheeler@ufl.edu
#SBATCH --account=eel6763
#SBATCH --qos=eel6763
#SBATCH --nodes=2
#SBATCH --ntasks=8
#SBATCH --ntasks-per-node=4
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=1000mb
#SBATCH -t 00:05:00
#SBATCH -o myoutput
#SBATCH -e myerr
srun --mpi=pmix_v3 ./mat_mult
