#!/bin/bash
	
mpirun -np 1 main.out 1 tests/NOT/in24.dat tests/NOT/t.dat < tests/NOT/stdin > /dev/null

for (( i=2; i <=4; i*=2))
do
	for (( j=2; j <=8; j*=2))
	do
		mpirun --oversubscribe -np $i main.out $j tests/NOT/in24.dat tests/NOT/out24.dat < tests/NOT/stdin > /dev/null && ./compare.out tests/NOT/out24.dat tests/NOT/t.dat
	done
done
