#!/bin/bash
	
mpirun -np 1 main.out 1 tests/H/in24.dat tests/H/t.dat < tests/H/stdin > /dev/null

for (( i=2; i <=4; i*=2))
do
	for (( j=2; j <=8; j*=2))
	do
		mpirun --oversubscribe -np $i main.out $j tests/H/in24.dat tests/H/out24.dat < tests/H/stdin > /dev/null && ./compare.out tests/H/out24.dat tests/H/t.dat
	done
done
