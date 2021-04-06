generate.out: src/generate.cpp
	mpic++ src/generate.cpp -o generate.out

printFile.out: src/printFile.cpp
	g++ src/printFile.cpp -o printFile.out

compare.out: src/compare.cpp
	g++ src/compare.cpp -o compare.out

qubit.out: src/main.cpp
	mpic++ src/main.cpp -o qubit.out

test: generate.out compare.out qubit.out
	mpirun generate.out 16 vectors/test && mpirun -np 1 qubit.out 16 1 vectors/test vectors/testRes && \
		for i in 2 4 8; do \
			mpirun --oversubscribe -np $$i qubit.out 16 1 vectors/test vectors/tRes && ./compare.out vectors/testRes vectors/tRes; \
		done

RUN = mpisubmit.pl -p

generate: generate.out
	for i in 25 26 27; do \
		mpisubmit.pl -p 8 generate.out $$i vectors/report$$i; \
	done

report: qubit.out generate.out generate
	for i in 1 2 4 8; do \
                for j in 25 26 27; do \
                        for k in 1 15 $$j; do \
                                $(RUN) $$i qubit.out $$j $$k vectors/report$$j; \
                        done \
                done \
        done


clean:
	rm *.out *.err report/*.dat vectors/*
