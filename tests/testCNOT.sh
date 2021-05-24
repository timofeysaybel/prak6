#!/bin/bash
	
mpirun -np 1 main.out 1 tests/CNOT/in20.dat tests/CNOT/t.dat < tests/CNOT/stdin > /dev/null

for (( i=2; i <=8; i*=2))
do
	mpirun --oversubscribe -np $i main.out 1 tests/CNOT/in20.dat tests/CNOT/out20.dat < tests/CNOT/stdin > /dev/null && ./compare.out tests/CNOT/out20.dat tests/CNOT/t.dat
done
