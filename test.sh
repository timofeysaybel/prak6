#!/bin/bash

for i in 1 2 4 8 
do
	for j in 20 24 28 30 
	do
        	for k in 1 15 $j
		do
                	./main.out $j $k $i >> report/report.dat
        	done
	done
done

