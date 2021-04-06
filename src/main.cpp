#include <mpi.h>
#include <cstdlib>
#include <fstream>
#include <complex>
#include <iostream>

typedef std::complex<double> complexd;

static int rank, size, log_size;

unsigned long long num_of_doubles(int n);

complexd* read(char *f, int *n);

complexd *qubitConvert(complexd *A, int n, int k, complexd *P);

void write(char *f, complexd *B, int n);


int main(int argc, char **argv)
{
	int n, k;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    for (log_size = 0; !((size >> log_size) & 1); log_size++);

    if (argc < 3)
    {
        if (!rank)
            std::cout << "Параметры командной строки: " << argv[0] << " n k infile (outfile)" << std::endl;

        MPI_Finalize();
        return 1;
    }

    n = atoi(argv[1]);
    k = atoi(argv[2]);

	double time, timeMAX;

	MPI_Barrier(MPI_COMM_WORLD);

	complexd *A = read(argv[3], &n);

	complexd P[] = {1 / sqrt(2), 1 / sqrt(2), 1 / sqrt(2), -1 / sqrt(2)};

	MPI_Barrier(MPI_COMM_WORLD);
	
	time = MPI_Wtime();
	complexd *B = qubitConvert(A, n, k, P);
	time = MPI_Wtime() - time;
	
	if (argc > 4)
		write(argv[4], B, n);

	delete[] B;
	MPI_Reduce(&time, &timeMAX, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	
	if (!rank)
	{
		std::ofstream file("report/data.dat",std::ios_base::app);
		file << n << "\t" << k << "\t" << size << "\t" << timeMAX << std::endl;
		file.close();
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

    unsigned long long m = num_of_doubles(*n);
    complexd *A = new complexd[m];
    double d[2];

    MPI_File_seek(file, sizeof(int) + 2 * m * rank * sizeof(double), MPI_SEEK_SET);
    for (unsigned long long i = 0; i < m; i++)
    {
        MPI_File_read(file, &d, 2, MPI_DOUBLE, MPI_STATUS_IGNORE);
        A[i].real(d[0]);
        A[i].imag(d[1]);
    }
    MPI_File_close(&file);
    return A;
}

complexd *qubitConvert(complexd *A, int n, int k, complexd *P)
{
    unsigned long long m = num_of_doubles(n);
    complexd *B = new complexd[m];
    if (k > log_size)
    {
        unsigned long long l = 1 << (n - k);
        for (unsigned long long i = 0; i < m; i++)
            B[i] = ((i & l) == 0) ? P[0] * A[i & ~l] + P[1] * A[i | l] : P[2] * A[i & ~l] + P[3] * A[i | l];
    }
    else
    {
        complexd *BUF = new complexd[m];
        int rank1 = rank ^ (1 << (log_size - k));
        MPI_Sendrecv(A, m, MPI_DOUBLE_COMPLEX, rank1, 0, BUF, m, MPI_DOUBLE_COMPLEX, rank1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (rank < rank1)
        {
            for (unsigned long long i = 0; i < m; ++i)
                B[i] = P[0] * A[i] + P[1] * BUF[i];
        }
        else
        {
            for (unsigned long long i = 0; i < m; ++i)
                B[i] = P[2] * BUF[i] + P[3] * A[i];
        }
        delete[] BUF;
    }
    return B;
}

unsigned long long num_of_doubles(int n)
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
    
    unsigned long long m = num_of_doubles(n);
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
