#!/bin/bash
	
mpirun -np 1 main.out 1 tests/CROT/in24.dat tests/CROT/t.dat < tests/CROT/stdin > /dev/null

for (( i=2; i <=4; i*=2))
do
	for (( j=2; j <=8; j*=2))
	do
		mpirun --oversubscribe -np $i main.out $j tests/CROT/in24.dat tests/CROT/out24.dat < tests/CROT/stdin > /dev/null && ./compare.out tests/CROT/out24.dat tests/CROT/t.dat
	done
done
