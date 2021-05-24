#!/bin/bash
	
mpirun -np 1 main.out 1 tests/H/in20.dat tests/H/t.dat < tests/H/stdin > /dev/null

for (( i=2; i <=8; i*=2))
do
	mpirun --oversubscribe -np $i main.out 1 tests/H/in20.dat tests/H/out20.dat < tests/H/stdin > /dev/null && ./compare.out tests/H/out20.dat tests/H/t.dat
done
