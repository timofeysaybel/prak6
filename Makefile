RUN = mpirun --oversubscribe -np
PL = mpisubmit.pl -p

all: compare.out generate.out printFile.out main.out

compare.out:
	g++ src/compare.cpp -o compare.out

generate.out:
	mpic++ src/generate.cpp -o generate.out

printFile.out:
	g++ src/printFile.cpp -o printFile.out

main.out:
	mpic++ src/main.cpp -o main.out -fopenmp

clean: 
	rm *.out tests/H/*.dat tests/nH/*.dat tests/NOT/*.dat tests/CNOT/*.dat tests/ROT/*.dat tests/CROT/*.dat report/*.dat

generate: generate.out
	for i in 20 24 28; do \
		$(PL) 1 generate.out $$i tests/H/in$$i.dat; \
	        $(PL) 1 generate.out $$i tests/nH/in$$i.dat; \
        	$(PL) 1 generate.out $$i tests/NOT/in$$i.dat; \
        	$(PL) 1 generate.out $$i tests/CNOT/in$$i.dat; \
        	$(PL) 1 generate.out $$i tests/ROT/in$$i.dat; \
        	$(PL) 1 generate.out $$i tests/CROT/in$$i.dat; \
	done

tGenerate: generate.out
	$(RUN) 1 generate.out 20 tests/H/in20.dat; \
	$(RUN) 1 generate.out 20 tests/nH/in20.dat; \
	$(RUN) 1 generate.out 20 tests/NOT/in20.dat; \
	$(RUN) 1 generate.out 20 tests/CNOT/in20.dat; \
	$(RUN) 1 generate.out 20 tests/ROT/in20.dat; \
	$(RUN) 1 generate.out 20 tests/CROT/in20.dat; \

testH: tGenerate compare.out main.out
	./tests/testH.sh

testnH: tGenerate compare.out main.out
	./tests/testnH.sh

testNOT: tGenerate compare.out main.out
	./tests/testNOT.sh

testCNOT: tGenerate compare.out main.out
	./tests/testCNOT.sh

testROT: tGenerate compare.out main.out
	./tests/testROT.sh

testCROT: tGenerate compare.out main.out
	./tests/testCROT.sh

tests: testH testnH testNOT testCNOT testROT testCROT

report: main.out generate
	for i in 20 24; do \
		for j in 1 2 4 8 16; do \
			$(PL) $$j main.out 1 tests/nH/in$$i.dat tests/nH/out$$i.dat report/nH.dat < tests/nH/stdin > /dev/null; \
			$(PL) $$j main.out 1 tests/CNOT/in$$i.dat tests/CNOT/out$$i.dat report/CNOT.dat < tests/CNOT/stdin > /dev/null; \
		done \
	done
