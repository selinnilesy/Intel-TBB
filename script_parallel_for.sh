#PBS -l select=1:node_type=rome:ncpus=4
#PBS -l walltime=00:10:00

cd $PBS_O_WORKDIR
module load tbb
make all
./parallel_for 1000000 4 silent 
