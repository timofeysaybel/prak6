#!/bin/bash
	
mpirun -np 1 main.out 1 tests/ROT/in24.dat tests/ROT/t.dat < tests/ROT/stdin > /dev/null

for (( i=2; i <=4; i*=2))
do
	for (( j=2; j <=8; j*=2))
	do
		mpirun --oversubscribe -np $i main.out $j tests/ROT/in24.dat tests/ROT/out24.dat < tests/ROT/stdin > /dev/null && ./compare.out tests/ROT/out24.dat tests/ROT/t.dat
	done
done
