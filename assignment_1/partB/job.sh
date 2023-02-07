#!/bin/bash
#SBATCH --job-name=monte_carlo_sendrcv
#SBATCH --mail-type=FAIL
#SBATCH --mail-user=benjaminwheeler@ufl.edu
#SBATCH --account=eel6763
#SBATCH --qos=eel6763
#SBATCH --nodes=1
#SBATCH --ntasks=8
#SBATCH --ntasks-per-node=8
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=1000mb
#SBATCH -t 00:01:00
#SBATCH -o myoutput
#SBATCH -e myerr
srun --mpi=pmix_v3 ./monte_carlo -10 10 100

