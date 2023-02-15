#!/bin/bash
#SBATCH --job-name=weighted_avg_filter
#SBATCH --mail-type=FAIL
#SBATCH --mail-user=benjaminwheeler@ufl.edu
#SBATCH --account=eel6763
#SBATCH --qos=eel6763
#SBATCH --nodes=2
#SBATCH --ntasks=4
#SBATCH --ntasks-per-node=2
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=1000mb
#SBATCH -t 00:01:00
#SBATCH -o myout
#SBATCH -e myerr
srun --mpi=pmix_v3 ./weighted_avg_filter 10
