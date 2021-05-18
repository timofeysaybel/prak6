#!/bin/bash
	
mpirun -np 1 main.out 1 tests/nH/in24.dat tests/nH/t.dat < tests/nH/stdin > /dev/null

for (( i=2; i <=4; i*=2))
do
	for (( j=2; j <=8; j*=2))
	do
		mpirun --oversubscribe -np $i main.out $j tests/nH/in24.dat tests/nH/out24.dat < tests/nH/stdin > /dev/null && ./compare.out tests/nH/out24.dat tests/nH/t.dat
	done
done
