#include <fstream>

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

	complex<double> *U = getRandVec(n);
	complex<double> P[] = {1/sqrt(2), 1/sqrt(2), 1/sqrt(2), -1/sqrt(2)};
	double time = omp_get_wtime();
    complex<double> *W = qubitConvert(U, n, P, k);
	time = omp_get_wtime() - time;
    
    /*ofstream fileU("result/U"), fileW("result/W");
	unsigned long long i, m = 1 << n;
	for (i = 0; i < m; i++) 
        fileU << U[i] << endl;
        
	for (i = 0; i < m; i++) 
        fileW << W[i] << endl;
	fileU.close();
    fileW.close();*/

    ofstream rep("report/report.dat",ios::app);
	rep << n << '\t' << k << '\t' << num << '\t' << time << endl;
	delete [] U;
	delete [] W;
    rep.close();
    return 0;
}
