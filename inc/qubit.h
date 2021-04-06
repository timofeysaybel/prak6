#include <complex>

typedef std::complex<double> complexd;

int rank, size, log_size;

unsigned long long num_of_doubles(int n);

complexd* read(char *f, int *n);

complexd *qubitConvert(complexd *A, int n, int k, complexd *P);

void write(char *f, complexd *B, int n);
