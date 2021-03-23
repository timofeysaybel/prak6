#include "../inc/qubit.h"

complex<double>* getRandVec(int n)
{
    unsigned long long i, m = 1 << n;
	complex<double> *res = new complex<double>[m];

    srand(time(NULL));

    for (i = 0; i < m; i++)
    {
        res[i].real(rand() / omp_get_wtime());
        res[i].imag(rand() / omp_get_wtime());
    }

    return res;
}

complex<double>* qubitConvert(complex<double> *U, int n, complex<double> *P, int k)
{
    unsigned long long i, m = 1 << n, l = 1 << (n - k);
	complex<double> *W = new complex<double>[m];
	
	#pragma omp parallel shared(U, W, P, m, l) private(i)
	{
		#pragma omp for schedule(guided)
		for (i = 0; i < m; i++)
        {
            if (i & l == 0)
                W[i] = P[0]*U[i & ~l] + P[1]*U[i | l];
            else
                W[i] = P[2]*U[i & ~l] + P[3]*U[i | l];
        }
	}
	
	return W;
}