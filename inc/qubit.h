#include <iostream>
#include <cstdlib>
#include <complex>
#include <cmath>
#include <omp.h>

std::complex<double>* getRandVec(int n);

std::complex<double>* qubitConvert(std::complex<double> *U, int n, std::complex<double> *P, int k);