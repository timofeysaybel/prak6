#!/bin/bash
	
mpirun -np 1 main.out 1 tests/nH/in20.dat tests/nH/t.dat < tests/nH/stdin > /dev/null

for (( i=2; i <=8; i*=2))
do
	mpirun --oversubscribe -np $i main.out 1 tests/nH/in20.dat tests/nH/out20.dat < tests/nH/stdin > /dev/null && ./compare.out tests/nH/out20.dat tests/nH/t.dat
done
