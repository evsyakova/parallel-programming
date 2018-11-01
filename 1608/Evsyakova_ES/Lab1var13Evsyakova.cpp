#include <iostream>
#include <iomanip>
#include <time.h>
#include <MPI.h>
using namespace std;

int MaxSearch(const int* vector, const int n) // принимает массив и его размер
{
	int max;
	if (n == 0) // если размер равен нулю, устанавливает размер равный максимальному значению int
		return INT_MAX;

	max = vector[0];
	for (int i = 0; i < n; i++) // если максимальный элемент матрицы меньше текущего
		if (max < vector[i])    // то текущий заменяет максимальный
			max = vector[i];

	return max;
}

void OutputMatr(const int* matrix, const int n, const int m) // выведение матрицы размеров n на m на экран
{
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < m; j++)
			cout << matrix[i * m + j];
		cout << endl;
	}
}

int main(int argc, char* argv[])
{
	int* matrix = 0;
	int rows, columns;
	int max;
	if (argc != 3) // если количество дополнительных параметров не равно двум (первый параметр - название программы)
	{              // то показывается, как надо ввести параметры
		cout << "Enter Matrix Sizes\nrows;" << endl;
		cin >> rows;
		cout << "columns:" << endl;
		cin >> columns;
		return 1;
	}
	else
	{
		rows = atoi(argv[1]); // принимаем параметры и конвертируем их в Int
		columns = atoi(argv[2]);
	}

	


	int proc_num, proc_rank; // количество процессов, номер процесса
	int *send_counts, *displs, *recieve_buffer; // количество элементов для отправки, смещение, буффер приема
	int chunk_size, rem; // размер куска, остаток

	double start, end;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &proc_num);
	MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);

	

	
	// Параллельная часть
	start = MPI_Wtime();
	send_counts = new int[proc_num];
	displs = new int[proc_num];
	chunk_size = rows * columns / proc_num; // расчитываем размер передаваемого куска
	rem = rows * columns % proc_num;        // расчитываем остаток если размер матрицы не делится нацело на количество процессов

	for (int i = 0; i < proc_num; i++) // устанавливаем размер и смещение передаваемого куска для каждого процесса
	{
		send_counts[i] = chunk_size;
		displs[i] = rem + i * chunk_size;
	}

	recieve_buffer = new int[send_counts[proc_rank]];
	// разбиваем матрицу и отправляем ее остальным процессам
	MPI_Scatterv(matrix, send_counts, displs, MPI_INT, recieve_buffer, send_counts[proc_rank], MPI_INT, 0, MPI_COMM_WORLD);
	/* (const void *sendbuf, const int *sendcounts, const int *displs,
	MPI_Datatype sendtype, void *recvbuf, int recvcount,
	MPI_Datatype recvtype, int root, MPI_Comm comm) */

	max = MaxSearch(recieve_buffer, send_counts[proc_rank]); // вычисляем максимум
	int GlobMax = 0;

	MPI_Reduce(&max, &GlobMax, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD); // собираем максимальные значения со всех процессов
	
	if (proc_rank == 0)
	{
		matrix = new int[rows * columns];
		srand(time(NULL));
		for (int i = 0; i < rows * columns; i++) // заполнение матрицы
			matrix[i] = rand() % 100;        // значениями от 0 до 100

		if ((rows < 10) && (columns < 10)) // если матрица меньше чем 10 на 10 то выводит ее на экран
			OutputMatr(matrix, rows, columns);
	}
	if (proc_rank == 0)
	{
		end = MPI_Wtime();

		cout << "max = " << GlobMax << endl;
		cout << "Parallel time : " << end - start << endl;

		// Линейная часть
		start = MPI_Wtime();
		max = MaxSearch(matrix, rows * columns); // вычисляем максимум
		end = MPI_Wtime();
		cout << fixed << "Linear time   : " << end - start << endl;

		if (GlobMax == max) // проверка на то что линейная и параллельная реализации посчитали одинакого
			cout << "Calculations are correct" << endl;
		else
			cout << "Calculations are wrong" << endl;

		delete[] matrix;
	}
	MPI_Finalize();

	delete[] send_counts;
	delete[] displs;
	delete[] recieve_buffer;
	getchar();

	return 0;
}