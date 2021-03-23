#include "../inc/qubit.h"

complex<double>* gen(int n)
{
    unsigned long long i, m = 1 << n;
	complex<double> *A = new complex<double>[m];
	
	double module = 0;
	unsigned int seed = omp_get_wtime();
	#pragma omp parallel shared(A, m) firstprivate(seed) private(i) reduction(+: module)
	{
		seed += omp_get_thread_num();
		#pragma omp for schedule(guided)
		for (i = 0; i < m; ++i) {
			A[i].real((rand_r(&seed) / (float) RAND_MAX) - 0.5f);
			A[i].imag((rand_r(&seed) / (float) RAND_MAX) - 0.5f);
			module += abs(A[i]*A[i]);
		}
	}
	module = sqrt(module);
	
	#pragma omp parallel for schedule(guided)
	for (i = 0; i < m; ++i) A[i] /= module;
	
	return A;
}

complex<double>* f(complex<double> *A, int n, complex<double> *P, int k)
{
    unsigned long long i, m = 1 << n, l = 1 << (n - k);
	complex<double> *B = new complex<double>[m];
	
	#pragma omp parallel shared(A, B, P, m, l) private(i)
	{
		#pragma omp for schedule(guided)
		for (i = 0; i < m; ++i)
			B[i] = ((i & l) == 0) ? P[0]*A[i & ~l] + P[1]*A[i | l] : P[2]*A[i & ~l] + P[3]*A[i | l];
	}
	
	return B;
}