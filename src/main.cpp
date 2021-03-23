#include "../inc/qubit.h"

int main(int argc,char **argv)
{
    if (argc < 4)
    {
        cerr << "Параметры командной строки: " << argv[0] << "n k num_threads" << endl;
        return -1;
    }

    int n, k, num;
	n = atoi(argv[1]);
	k = atoi(argv[2]);
    num = atoi(argv[3]);
    omp_set_num_threads(num);

	complex<double> *A = gen(n);
	double time = omp_get_wtime();
	complex<double> P[] = {1/sqrt(2), 1/sqrt(2), 1/sqrt(2), -1/sqrt(2)};
	complex<double> *B = f(A, n, P, k);
	time = omp_get_wtime() - time;
    
    /*
	unsigned long long i, m = 1LLU << n;
	for (i = 0; i < m; ++i) cout << A[i] << endl;
	cout << endl;
	for (i = 0; i < m; ++i) cout << B[i] << endl;
	*/

	cout << n << '\t' << k << '\t' << num << '\t' << time << endl;
	delete [] A;
	delete [] B;

    return 0;
}
