#!/bin/bash
	
mpirun -np 1 main.out 1 tests/CNOT/in24.dat tests/CNOT/t.dat < tests/CNOT/stdin > /dev/null

for (( i=2; i <=4; i*=2))
do
	for (( j=2; j <=8; j*=2))
	do
		mpirun --oversubscribe -np $i main.out $j tests/CNOT/in24.dat tests/CNOT/out24.dat < tests/CNOT/stdin > /dev/null && ./compare.out tests/CNOT/out24.dat tests/CNOT/t.dat
	done
done
