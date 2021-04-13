#!/bin/bash
for i in 0.1 0.01 0.001
do
	for (( j=1; j<=60; j++))
	do
		mpisubmit.bg 8 qubit.out 26 $i 8 vectors/report26 vectors/t report/fixedN$i
	done
done
