#include <omp.h>
#include <mpi.h>
#include <complex>
#include <map>

typedef std::complex<double> complexd;

namespace Quantum
{
    static bool initFlag = false;
    static int rank = 0, size = 0, logSize = 0, threads = 0;

    static inline void init()
    {
        if (initFlag)
            return;

        int status;
        MPI_Initialized(&status);
        if (!status)
            throw 1;

        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &size);
        threads = omp_get_num_threads();

        for (logSize = 0; ((size >> logSize) & 1) == 0; logSize++)
            ;

        if ((size >> logSize) != 1)
            throw 2;

        initFlag = true;
    }

    static inline unsigned long long numOfDoubles(int n)
    {
        if (logSize > n)
            return 0;
        return 1 << (n - logSize);
    }

    static inline unsigned long long *getMasks(unsigned int i, const unsigned int *k, unsigned int len)
    {
        unsigned int l = 1 << i;
        unsigned long long *masks = new unsigned long long[l];
        for (unsigned int j = 0; j < l; j++)
        {
            masks[j] = 0;
            for (unsigned int p = 0; p < i; p++)
                if (((j >> p) & 1) == 1)
                    masks[j] ^= 1 << (len - k[i - 1 - p]);
        }
        return masks;
    }

    static inline unsigned int *getRanks(unsigned int i, const unsigned int *k)
    {
        unsigned int p[i], j = 0;

        for (unsigned int t = 0; t < i; t++)
            if (k[t] <= (unsigned int)logSize)
                p[j++] = k[t];

        unsigned int *ranks = reinterpret_cast<unsigned int *>(getMasks(j, p, logSize));

        ranks[0] = 1 << j;
        for (unsigned int t = 1; t < ranks[0]; t++)
            ranks[t] ^= rank;
        --ranks[0];
        return ranks;
    }

    static inline complexd *read(const char *f, int *n)
    {
        MPI_File file;
        if (MPI_File_open(MPI_COMM_WORLD, f, MPI_MODE_RDONLY, MPI_INFO_NULL, &file))
            throw 4;

        *n = 0;
        if (!rank)
            MPI_File_read(file, (unsigned char *)n, 1, MPI_UNSIGNED_CHAR, MPI_STATUS_IGNORE);

        MPI_Bcast(n, 1, MPI_INT, 0, MPI_COMM_WORLD);
        unsigned long long m = numOfDoubles(*n);
        if (m == 0)
        {
            MPI_File_close(&file);
            throw 3;
        }
        complexd *A = new complexd[m];
        MPI_File_seek(file, sizeof(unsigned char) + m * sizeof(complexd) * rank, MPI_SEEK_SET);
        MPI_File_read(file, A, m, MPI_CXX_DOUBLE_COMPLEX, MPI_STATUS_IGNORE);
        MPI_File_close(&file);
        return A;
    }

    static inline void write(const char *f, const complexd *A, unsigned int n)
    {
        MPI_File file;
        if (MPI_File_open(MPI_COMM_WORLD, f, MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &file))
        {
            throw 4;
        }

        if (!rank)
            MPI_File_write(file, (unsigned char *)&n, 1, MPI_UNSIGNED_CHAR, MPI_STATUS_IGNORE);

        unsigned long long m = numOfDoubles(n);
        if (m == 0)
        {
            MPI_File_close(&file);
            throw 3;
        }
        MPI_File_seek(file, sizeof(unsigned char) + m * sizeof(complexd) * rank, MPI_SEEK_SET);
        MPI_File_write(file, A, m, MPI_CXX_DOUBLE_COMPLEX, MPI_STATUS_IGNORE);
        MPI_File_close(&file);
    }

    static inline int convert(const complexd *A, complexd *B, unsigned int n, unsigned int i, complexd **P, const unsigned int *k, complexd *BUF = nullptr)
    {
        unsigned long long m = numOfDoubles(n);
        if (m == 0)
            throw 3;

        unsigned int l = 1 << i;
        unsigned long long *masks = getMasks(i, k, n);
        unsigned int *ranks = getRanks(i, k);

        bool create_buf_flag = false;
        if ((BUF == nullptr) && (ranks[0] > 0))
        {
            BUF = new complexd[m * ranks[0]];
            create_buf_flag = true;
        }

        std::map<unsigned int, unsigned int> rank2id;
        for (unsigned int j = 0; j < ranks[0]; j++)
        {
            MPI_Sendrecv(A, m, MPI_CXX_DOUBLE_COMPLEX, ranks[j + 1], 0, BUF + m * j, m, MPI_CXX_DOUBLE_COMPLEX, ranks[j + 1], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            rank2id[ranks[j + 1]] = j;
        }

#pragma omp parallel
        {
            complexd *vec = new complexd[l];
            unsigned int x = 0, id0;          // P line number, rank id
            unsigned long long tmp, num, id1; // element number, local id
            const unsigned long long tmp0 = rank << (n - logSize),
                                     tmp1 = ~((~0) << (n - logSize));

#pragma omp for schedule(guided)
            for (unsigned long long j = 0; j < m; ++j)
            {
                tmp = (tmp0 ^ j) & ~masks[l - 1];
                for (unsigned int z = 0; z < l; ++z)
                {
                    num = tmp ^ masks[z];
                    id0 = num >> (n - logSize);
                    id1 = num & tmp1;
                    if (id0 == (unsigned int)rank)
                    {
                        vec[z] = A[id1];
                        if (id1 == j)
                            x = z;
                    }
                    else
                    {
                        vec[z] = BUF[m * rank2id[id0] + id1];
                    }
                }
                B[j] = 0;
                for (unsigned int z = 0; z < l; ++z)
                {
                    B[j] += P[x][z] * vec[z];
                }
            }
            delete[] vec;
        }

        if (create_buf_flag)
            delete[] BUF;

        delete[] masks;
        delete[] ranks;

        return 0;
    }

    static inline complexd *qubitConvert(const complexd *A, unsigned int n, unsigned int i, complexd **P, const unsigned int *k)
    {
        unsigned long long m = numOfDoubles(n);
        if (m == 0)
            throw 3;

        complexd *B = new complexd[m];
        convert(A, B, n, i, P, k);
        return B;
    }

    static inline complexd *Hadamard(const complexd *A, unsigned int n, unsigned int k)
    {
        complexd P0[] = {1 / sqrt(2), 1 / sqrt(2)};
        complexd P1[] = {1 / sqrt(2), -1 / sqrt(2)};
        complexd *P[] = {P0, P1};
        return qubitConvert(A, n, 1, P, &k);
    }

    static inline complexd *nHadamard(const complexd *A, unsigned int n)
    {
        unsigned long long m = numOfDoubles(n);
        if (m == 0)
            throw 3;

        complexd *B = new complexd[m], *C = new complexd[m], *T, *buf = new complexd[m];

#pragma omp parallel for schedule(guided)
        for (unsigned long long i = 0; i < m; i++)
        {
            B[i] = A[i];
        }

        complexd P0[] = {1 / sqrt(2), 1 / sqrt(2)};
        complexd P1[] = {1 / sqrt(2), -1 / sqrt(2)};
        complexd *P[] = {P0, P1};
        for (unsigned int k = 1; k <= n; k++)
        {
            convert(B, C, n, 1, P, &k, buf);
            T = B;
            B = C;
            C = T;
        }
        delete[] C;
        delete[] buf;
        return B;
    }

    static inline complexd *ROT(const complexd *A, unsigned int n, unsigned int k, double a)
    {
        complexd P0[] = {1, 0};
        complexd P1[] = {0, cos(a)};
        P1[1].imag(sin(a));
        complexd *P[] = {P0, P1};
        return qubitConvert(A, n, 1, P, &k);
    }

    static inline complexd *NOT(const complexd *A, unsigned int n, unsigned int k)
    {
        complexd P0[] = {0, 1};
        complexd P1[] = {1, 0};
        complexd *P[] = {P0, P1};
        return qubitConvert(A, n, 1, P, &k);
    }

    static inline complexd *CNOT(const complexd *A, unsigned int n, unsigned int k1, unsigned int k2)
    {
        unsigned int k[] = {k1, k2};
        complexd P0[] = {1, 0, 0, 0};
        complexd P1[] = {0, 1, 0, 0};
        complexd P2[] = {0, 0, 0, 1};
        complexd P3[] = {0, 0, 1, 0};
        complexd *P[] = {P0, P1, P2, P3};
        return qubitConvert(A, n, 2, P, k);
    }

    static inline complexd *CROT(const complexd *A, unsigned int n, unsigned int k1, unsigned int k2, double a)
    {
        unsigned int k[] = {k1, k2};
        complexd P0[] = {1, 0, 0, 0};
        complexd P1[] = {0, 1, 0, 0};
        complexd P2[] = {0, 0, 1, 0};
        complexd P3[] = {0, 0, 0, cos(a)};
        P3[3].imag(sin(a));
        complexd *P[] = {P0, P1, P2, P3};
        return qubitConvert(A, n, 2, P, k);
    }
};
