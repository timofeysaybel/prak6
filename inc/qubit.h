#include <iostream>
#include <cstdlib>
#include <complex>
#include <cmath>
#include <omp.h>

using namespace std;

complex<double>* getRandVec(int n);

complex<double>* qubitConvert(complex<double> *U, int n, complex<double> *P, int k);