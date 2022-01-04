all: reduce  parallel_for task reduce_optimized

reduce_optimized: reduce_optimized.cpp
	g++   reduce_optimized.cpp -o reduce_optimized  -I/opt/hlrs/non-spack/compiler/intel/19.1.0.166/compilers_and_libraries_2020.0.166/linux/tbb/include  -L/opt/hlrs/non-spack/compiler/intel/19.1.0.166/compilers_and_libraries_2020.0.166/linux/tbb/lib/intel64/gcc4.8  -pthread -ltbb 


task:	eratosthenes_tbb_task.cpp
	g++   eratosthenes_tbb_task.cpp -o task  -I/opt/hlrs/non-spack/compiler/intel/19.1.0.166/compilers_and_libraries_2020.0.166/linux/tbb/include  -L/opt/hlrs/non-spack/compiler/intel/19.1.0.166/compilers_and_libraries_2020.0.166/linux/tbb/lib/intel64/gcc4.8  -pthread -ltbb 


reduce : eratosthenes_tbb_reduce.cpp
	g++  eratosthenes_tbb_reduce.cpp -o reduce  -I/opt/hlrs/non-spack/compiler/intel/19.1.0.166/compilers_and_libraries_2020.0.166/linux/tbb/include  -L/opt/hlrs/non-spack/compiler/intel/19.1.0.166/compilers_and_libraries_2020.0.166/linux/tbb/lib/intel64/gcc4.8  -pthread -ltbb

parallel_for: eratosthenes_tbb.cpp
	g++ eratosthenes_tbb.cpp -o parallel_for  -I/opt/hlrs/non-spack/compiler/intel/19.1.0.166/compilers_and_libraries_2020.0.166/linux/tbb/include  -L/opt/hlrs/non-spack/compiler/intel/19.1.0.166/compilers_and_libraries_2020.0.166/linux/tbb/lib/intel64/gcc4.8  -pthread -ltbb

clean :
	rm reduce
	rm parallel_for 
	rm task
	rm reduce_optimized
