#include <iostream>
#include <omp.h>
#include <mpi.h>
#include <complex>
#include <fstream>

#include "../inc/quantum.h"

int main(int argc, char **argv)
{
    Quantum::initFlag = false;
    int rank, n;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (argc < 4)
    {
        if (!rank)
            std::cerr << argv[0] << "threads in out [report]" << std::endl;
        MPI_Finalize();
        return 0;
    }

    omp_set_num_threads(atoi(argv[1]));
    
    try
    {
        complexd *A = Quantum::read(argv[2], &n), *B = nullptr;

        int t, k[2];
        double a;
        if (!rank)
        {
            std::cout << "Количество процессов: " << Quantum::size << std::endl;
            std::cout << "Количество потоков: " << Quantum::threads << std::endl;
            std::cout << "Введите gate" << std::endl;
            std::cout << "1\tH" << std::endl;
            std::cout << "2\tH^n" << std::endl;
            std::cout << "3\tROT" << std::endl;
            std::cout << "4\tNOT" << std::endl;
            std::cout << "5\tCNOT" << std::endl;
            std::cout << "6\tCROT" << std::endl;
            std::cin >> t;
        }

        MPI_Bcast(&t, 1, MPI_INT, 0, MPI_COMM_WORLD);
        if (t == 1 || t == 3 || t == 4)
        {
            if (!rank)
            {
                std::cout << "Введите k: ";
                std::cin >> k[0];
            }
            MPI_Bcast(k, 1, MPI_INT, 0, MPI_COMM_WORLD);
        }
        if (t == 5 || t == 6)
        {
            if (!rank)
            {
                std::cout << "Введите k1 и k2: ";
                std::cin >> k[0] >> k[1];
            }
            MPI_Bcast(k, 2, MPI_INT, 0, MPI_COMM_WORLD);
        }
        if (t == 3 || t == 6)
        {
            if (!rank)
            {
                std::cout << "Введите a: ";
                std::cin >> a;
            }
            MPI_Bcast(&a, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        }
        double time, timeMAX;
        time = MPI_Wtime();
        switch(t)
        {
            case 1:
                B = Quantum::Hadamard(A, n, k[0]);
                break;

            case 2:
                B = Quantum::nHadamard(A, n);
                break;

            case 3:
                B = Quantum::ROT(A, n, k[0], a);
                break;

            case 4:
                B = Quantum::NOT(A, n, k[0]);
                break;

            case 5:
                B = Quantum::CNOT(A, n, k[0], k[1]);
                break;

            case 6:
                B = Quantum::CROT(A, n, k[0], k[1], a);
                break;

            default:
                break;
        }
        time = MPI_Wtime() - time;
        MPI_Reduce(&time, &timeMAX, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
        if (!rank && argc == 5)
        {
            std::ofstream file(argv[4], std::ios_base::app);
            file << n << "\t" << Quantum::size << "\t" << Quantum::threads << "\t" << timeMAX << std::endl;
            file.close();
        }
        Quantum::write(argv[3], B, n);
        delete[] A;
        delete[] B;
    }
    catch(int i)
    {
        switch(i)
        {
            case 1:
                std::cerr << "Функция была вызвана до MPI_Init" << std::endl;
                MPI_Finalize(); 
                return i;
            case 2:
                std::cerr << "Только для 2^n процессов" << std::endl;
                MPI_Finalize(); 
                return i;
            case 3:
                std::cerr << "Вектор слишком большой или слишком много процессов" << std::endl;
                MPI_Finalize(); 
                return i;
            case 4:
                std::cerr << "Не удалось открыть файл" << std::endl;
                MPI_Finalize(); 
                return i;
            default:
                std::cerr << "Возникла ошибка, не поддающаяся никакому объяснению" << std::endl;
                MPI_Finalize(); 
                return i;
        }
    }
    MPI_Finalize();
    return 0;
}