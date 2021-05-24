#!/bin/bash
	
mpirun -np 1 main.out 1 tests/CROT/in20.dat tests/CROT/t.dat < tests/CROT/stdin > /dev/null

for (( i=2; i <=8; i*=2))
do
	mpirun --oversubscribe -np $i main.out 1 tests/CROT/in20.dat tests/CROT/out20.dat < tests/CROT/stdin > /dev/null && ./compare.out tests/CROT/out20.dat tests/CROT/t.dat
done
