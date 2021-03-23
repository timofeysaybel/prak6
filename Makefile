main.out:
	g++ src/main.cpp src/qubit.cpp -o main.out -fopenmp

clean:
	rm *.out report/*.dat
RUN = mpisubmit-new.pl -p 

report: main.out
	for i in 1 2 4 8; do \
		for j in 20 24 28 30; do \
			for k in 1 15 $$j; do \
				$(RUN) i main.out $$j $$k; \
			done \
		done \
	done

rep: main.out
	./test.sh

