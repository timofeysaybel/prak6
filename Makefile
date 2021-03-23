main.out:
	g++ src/main.cpp src/qubit.cpp -o main.out -fopenmp

clean:
	rm *.out report/*.dat *.err
RUN = mpisubmit.pl 

report: main.out
	for i in 1 2 4 8; do \
		for j in 20 24 28 33; do \
			for k in 1 15 $$j; do \
				$(RUN) main.out $$j $$k $$i; \
			done \
		done \
	done


