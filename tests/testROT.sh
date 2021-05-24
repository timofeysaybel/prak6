#!/bin/bash
	
mpirun -np 1 main.out 1 tests/ROT/in20.dat tests/ROT/t.dat < tests/ROT/stdin > /dev/null

for (( i=2; i <=8; i*=2))
do
	mpirun --oversubscribe -np $i main.out 1 tests/ROT/in20.dat tests/ROT/out20.dat < tests/ROT/stdin > /dev/null && ./compare.out tests/ROT/out20.dat tests/ROT/t.dat
done
