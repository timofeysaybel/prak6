#include <complex>

typedef std::complex<double> complexd;

class Quantum
{
public:
    static bool initFlag;
	static int rank, size, logSize, threads;

    static complexd *read(const char *f, int *n);

    static void write(const char *f, const complexd *A, unsigned int n);

    static int convert(const complexd *A, complexd *B, unsigned int n, unsigned int i, complexd **P, const unsigned int *k, complexd *BUF = nullptr);

    static complexd *qubitConvert(const complexd *A, unsigned int n, unsigned int i, complexd **P, const unsigned int *k);

    static complexd *Hadamard(const complexd *A, unsigned int n, unsigned int k);

    static complexd *nHadamard(const complexd *A, unsigned int n);

    static complexd *ROT(const complexd *A, unsigned int n, unsigned int k, double a);

    static complexd *NOT(const complexd *A, unsigned int n, unsigned int k);

    static complexd *CNOT(const complexd *A, unsigned int n, unsigned int k1, unsigned int k2);

    static complexd *CROT(const complexd *A, unsigned int n, unsigned int k1, unsigned int k2, double a);

    static void init();
    static unsigned long long numOfDoubles(int n);
    static unsigned long long *getMasks(unsigned int i, const unsigned int *k, unsigned int len);
    static unsigned int *getRanks(unsigned int i, const unsigned int *k);
};
