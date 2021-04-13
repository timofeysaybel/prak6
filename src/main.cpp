#include <cstdlib>
#include <complex>
#include <mpi.h>
#include <omp.h>
#include <fstream>
#include <iostream>

typedef std::complex<double> complexd;
static int rank, size, log_size, threads;

double normal_dis_gen(unsigned int *seed);

int init(int argc, char **argv, int *n, double *e);

complexd *qubitHadamard(complexd *A, int n, double e);

unsigned long long numOfDoubles(int n);

complexd dot(complexd *A, complexd *B, int n);

///Чтение входного вектора из файла f
complexd *read(char *f, int *n);

///Однокубитное квантовое преобразование
complexd *qubitConvert(complexd *A, complexd *B, int n, int k, complexd *P, complexd *BUF);

///Запись результата B в файл f
void write(char *f, complexd *B, int n);

int main(int argc, char **argv)
{
    int n;
    double e;
    if (init(argc, argv, &n, &e))
        return 0;

    double time, timeMAX;
    MPI_Barrier(MPI_COMM_WORLD);

    complexd *A = read(argv[4], &n);

    MPI_Barrier(MPI_COMM_WORLD);
    time = MPI_Wtime();
    complexd *B = qubitHadamard(A, n, e);
    time = MPI_Wtime() - time;

    complexd *C = qubitHadamard(A, n, 0.0);

    delete[] A;

    MPI_Barrier(MPI_COMM_WORLD);
    double lost = abs(dot(B, C, n));
    if (!rank)
        std::cout << lost << std::endl;
    lost = 1.0 - lost * lost;
    if (lost < 0.0)
        lost = 0.0;

    if (argc > 5)
        write(argv[5], B, n);

    delete[] B;
    delete[] C;
    MPI_Reduce(&time, &timeMAX, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    if (!rank)
    {
        std::ofstream file(argv[6], std::ios_base::app);
        file << size << "\t" << threads << "\t" << n << "\t" << e << "\t" << lost << "\t" << timeMAX << std::endl;
    }
    MPI_Finalize();
    return 0;
}

complexd *read(char *f, int *n)
{
    MPI_File file;
    if (MPI_File_open(MPI_COMM_WORLD, f, MPI_MODE_RDONLY, MPI_INFO_NULL, &file))
    {
        if (!rank)
            std::cout << "Не удалось открыть файл " << f << std::endl;

        MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OTHER);
    }
    if (!rank)
        MPI_File_read(file, n, 1, MPI_INT, MPI_STATUS_IGNORE);

    MPI_Bcast(n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    unsigned long long m = numOfDoubles(*n);
    complexd *A = new complexd[m];
    double d[2];

    MPI_File_seek(file, sizeof(int) + 2 * m * rank * sizeof(double), MPI_SEEK_SET);
    for (unsigned long long i = 0; i < m; i++)
    {
        MPI_File_read(file, &d, 2, MPI_DOUBLE, MPI_STATUS_IGNORE);
        complexd tmp(d[0], d[1]);
        A[i] = tmp;
        //A[i].real(d[0]);
        //A[i].imag(d[1]);
    }
    MPI_File_close(&file);
    return A;
}

complexd *qubitConvert(complexd *A, complexd *B, int n, int k, complexd *P, complexd *BUF)
{
    unsigned long long m = numOfDoubles(n);
    if (k > log_size)
    {
        unsigned long long l = 1 << (n - k);
    #pragma omp parallel for schedule(guided)
        for (long long i = 0; i < m; i++)
        {
            B[i] = ((i & l) == 0) ? P[0] * A[i & ~l] + P[1] * A[i | l] : P[2] * A[i & ~l] + P[3] * A[i | l];
        }
    }
    else
    {
        int rank1 = rank ^ (1 << (log_size - k));
        MPI_Sendrecv(A, m, MPI_DOUBLE_COMPLEX, rank1, 0, BUF, m, MPI_DOUBLE_COMPLEX, rank1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (rank < rank1)
        {
    #pragma omp parallel for schedule(guided)
            for (long long i = 0; i < m; i++)
            {
                B[i] = P[0] * A[i] + P[1] * BUF[i];
            }
        }
        else
        {
    #pragma omp parallel for schedule(guided)
            for (long long i = 0; i < m; i++)
            {
                B[i] = P[2] * BUF[i] + P[3] * A[i];
            }
        }
    }
    return B;
}

unsigned long long numOfDoubles(int n)
{
    if (log_size > n)
    {
        if (!rank)
            std::cout << "Слишком много процессов для указанного n" << std::endl;

        MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OTHER);
    }
    return 1 << (n - log_size);
}

void write(char *f, complexd *B, int n)
{
    MPI_File file;
    if (MPI_File_open(MPI_COMM_WORLD, f, MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &file))
    {
        if (!rank)
            std::cout << "Не удалось открыть файл " << f << std::endl;

        MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OTHER);
    }
    if (!rank)
        MPI_File_write(file, &n, 1, MPI_INT, MPI_STATUS_IGNORE);

    unsigned long long m = numOfDoubles(n);
    double d[2];
    MPI_File_seek(file, sizeof(int) + 2 * m * rank * sizeof(double), MPI_SEEK_SET);

    for (unsigned long long i = 0; i < m; i++)
    {
        d[0] = B[i].real();
        d[1] = B[i].imag();
        MPI_File_write(file, &d, 2, MPI_DOUBLE, MPI_STATUS_IGNORE);
    }
    MPI_File_close(&file);
}

double normal_dis_gen()
{
    double S = 0.0;
    for (int i = 0; i < 12; i++)
    {
        S += (double)rand() / RAND_MAX;
    }
    return S - 6.0;
}

complexd *qubitHadamard(complexd *A, int n, double e)
{
    unsigned long long m = numOfDoubles(n);
    complexd *B = new complexd[m], *C = new complexd[m], *T, *buf = new complexd[m], P[4];

    #pragma omp parallel for schedule(guided)
    for (long long i = 0; i < m; i++)
    {
        B[i] = A[i];
    }
    double t;
    srand(time(NULL));

    for (int k = 1; k <= n; k++)
    {
        if (!rank)
        {
            t = normal_dis_gen();
            P[0] = (cos(e * t) - sin(e * t)) / sqrt(2);
            P[1] = P[2] = (cos(e * t) + sin(e * t)) / sqrt(2);
            P[3] = -P[0];
        }
        MPI_Bcast(P, 4, MPI_DOUBLE_COMPLEX, 0, MPI_COMM_WORLD);
        qubitConvert(B, C, n, k, P, buf);
        T = B;
        B = C;
        C = T;
    }
    delete[] C;
    delete[] buf;
    return B;
}

complexd dot(complexd *A, complexd *B, int n)
{
    unsigned long long m = numOfDoubles(n);
    complexd x(0.0, 0.0), y(0.0, 0.0);

    #pragma omp parallel
    {
        complexd z(0.0, 0.0);
        #pragma omp for schedule(guided)
        for (long long i = 0; i < m; i++)
        {
            z += conj(A[i]) * B[i];
        }

        #pragma omp critical
        {
            y += z;
        }
    }

    MPI_Reduce(&y, &x, 1, MPI_DOUBLE_COMPLEX, MPI_SUM, 0, MPI_COMM_WORLD);
	MPI_Bcast(&x, 1, MPI_DOUBLE_COMPLEX, 0, MPI_COMM_WORLD);
    return x;
}

int init(int argc, char **argv, int *n, double *e)
{
    int i = MPI_Init(&argc, &argv);
    if (i != MPI_SUCCESS)
    {
        std::cerr << "Error" << std::endl;
        return i;
    }
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    threads = omp_get_num_threads();

    for (log_size = 0; !((size >> log_size) & 1); ++log_size)
        ;

    if (argc < 3)
    {
        if (!rank)
            std::cout << "Параметры командной строки: " << argv[0] << " n e threads infile (outfile) report" << std::endl;

        MPI_Finalize();
        return 1;
    }
    *n = atoi(argv[1]);
    *e = atof(argv[2]);
    threads = atoi(argv[3]);
    omp_set_num_threads(threads);
    return 0;
}
