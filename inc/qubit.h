#include <iostream>
#include <cstdlib>
#include <complex>
#include <cmath>
#include <omp.h>

using namespace std;

complex<double>* gen(int n);

complex<double>* f(complex<double> *A, int n, complex<double> *P, int k);