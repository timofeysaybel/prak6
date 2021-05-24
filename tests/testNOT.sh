#!/bin/bash
	
mpirun -np 1 main.out 1 tests/NOT/in20.dat tests/NOT/t.dat < tests/NOT/stdin > /dev/null

for (( i=2; i <=8; i*=2))
do
	mpirun --oversubscribe -np $i main.out 1 tests/NOT/in20.dat tests/NOT/out20.dat < tests/NOT/stdin > /dev/null && ./compare.out tests/NOT/out20.dat tests/NOT/t.dat
done
