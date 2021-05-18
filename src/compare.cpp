#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>

using namespace std;

int main(int argc, char **argv)
{

	if (argc < 3)
	{
		cout << "Параметры командной строки: " << argv[0] << " A B [epsilon]" << endl;
		return 0;
	}

	double epsilon = 1e-3;
	
	if (argc >= 4)
		epsilon = atof(argv[3]);

	ifstream f1(argv[1], ios::binary), f2(argv[2], ios::binary);
	if (!f1.is_open())
	{
		cout << "Error opening file " << argv[1] << endl;
		return 1;
	}
	if (!f2.is_open())
	{
		cout << "Error opening file " << argv[2] << endl;
		return 1;
	}

	int n, n1;
	f1.read((char *)&n, sizeof(int));
	f2.read((char *)&n1, sizeof(int));
	if (n != n1)
	{
		cout << "Вектора разных размерностей" << endl;
		return 0;
	}

	unsigned long long m = 1 << n, i;
	double x[2], y[2];
	for (i = 0; i < m; ++i)
	{
		f1.read((char *)&x, 2 * sizeof(double));
		f2.read((char *)&y, 2 * sizeof(double));
		if ((abs(x[0] - y[0]) > epsilon) || (abs(x[1] - y[1]) > epsilon))
		{
			cout << "Некорректно: элементы вектора отличаются более чем на " << scientific << epsilon << endl;
			f1.close();
			f2.close();
			return 0;
		}
	}

	cout << "Корректно" << endl;
	f1.close();
	f2.close();
	return 0;
}

