generate.out: src/generate.cpp
	mpicxx src/generate.cpp -o generate.out

printFile.out: src/printFile.cpp
	g++ src/printFile.cpp -o printFile.out

compare.out: src/compare.cpp
	g++ src/compare.cpp -o compare.out

qubit.out: src/main.cpp
	mpixlcxx_r src/main.cpp -o qubit.out 

qubitxx.out: src/main.cpp
	mpic++ src/main.cpp -o qubit.out -fopenmp

test: generate.out compare.out qubitxx.out
	mpirun generate.out 16 vectors/test && mpirun -np 1 qubit.out 16 0.01 1 vectors/test vectors/testRes report/test.dat && \
		for i in 2 4 8; do \
			mpirun --oversubscribe -np $$i qubit.out 16 0.01 1 vectors/test vectors/tRes report/test.dat && ./compare.out vectors/testRes vectors/tRes; \
		done

RUN = mpisubmit.bg -n

generate: generate.out
	for i in 24 25 26 27 28; do \
		$(RUN) 8 generate.out $$i vectors/report$$i; \
	done

report: qubit.out generate
	for i in 1 2 4; do \
        	for k in 1 2 4 8; do \
	                $(RUN) $$i qubit.out 28 0.01 $$k vectors/report28 vectors/t report/data.dat; \
                done \
        done

fixedE: qubit.out generate
	./fixedE.sh

fixedN: qubit.out generate
	./fixedN.sh

clean:
	rm *.out *.err report/*.dat vectors/*
