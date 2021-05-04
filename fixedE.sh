#!/bin/bash
	
for (( i=24; i <=28; i++))
do
	for (( j=1; j <=60; j++))
	do
		mpisubmit.bg -n 8 qubit.out $i 0.01 8 vectors/report$i vectors/t report/fixedE$i
	done
done	
