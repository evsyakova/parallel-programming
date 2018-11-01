#include <iostream>
#include <iomanip>
#include <time.h>
#include <MPI.h>
using namespace std;

int MaxSearch(const int* vector, const int n) // ��������� ������ � ��� ������
{
	int max;
	if (n == 0) // ���� ������ ����� ����, ������������� ������ ������ ������������� �������� int
		return INT_MAX;

	max = vector[0];
	for (int i = 0; i < n; i++) // ���� ������������ ������� ������� ������ ��������
		if (max < vector[i])    // �� ������� �������� ������������
			max = vector[i];

	return max;
}

void OutputMatr(const int* matrix, const int n, const int m) // ��������� ������� �������� n �� m �� �����
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
	if (argc != 3) // ���� ���������� �������������� ���������� �� ����� ���� (������ �������� - �������� ���������)
	{              // �� ������������, ��� ���� ������ ���������
		cout << "Enter Matrix Sizes\nrows;" << endl;
		cin >> rows;
		cout << "columns:" << endl;
		cin >> columns;
		return 1;
	}
	else
	{
		rows = atoi(argv[1]); // ��������� ��������� � ������������ �� � Int
		columns = atoi(argv[2]);
	}

	


	int proc_num, proc_rank; // ���������� ���������, ����� ��������
	int *send_counts, *displs, *recieve_buffer; // ���������� ��������� ��� ��������, ��������, ������ ������
	int chunk_size, rem; // ������ �����, �������

	double start, end;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &proc_num);
	MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);

	

	
	// ������������ �����
	start = MPI_Wtime();
	send_counts = new int[proc_num];
	displs = new int[proc_num];
	chunk_size = rows * columns / proc_num; // ����������� ������ ������������� �����
	rem = rows * columns % proc_num;        // ����������� ������� ���� ������ ������� �� ������� ������ �� ���������� ���������

	for (int i = 0; i < proc_num; i++) // ������������� ������ � �������� ������������� ����� ��� ������� ��������
	{
		send_counts[i] = chunk_size;
		displs[i] = rem + i * chunk_size;
	}

	recieve_buffer = new int[send_counts[proc_rank]];
	// ��������� ������� � ���������� �� ��������� ���������
	MPI_Scatterv(matrix, send_counts, displs, MPI_INT, recieve_buffer, send_counts[proc_rank], MPI_INT, 0, MPI_COMM_WORLD);
	/* (const void *sendbuf, const int *sendcounts, const int *displs,
	MPI_Datatype sendtype, void *recvbuf, int recvcount,
	MPI_Datatype recvtype, int root, MPI_Comm comm) */

	max = MaxSearch(recieve_buffer, send_counts[proc_rank]); // ��������� ��������
	int GlobMax = 0;

	MPI_Reduce(&max, &GlobMax, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD); // �������� ������������ �������� �� ���� ���������
	
	if (proc_rank == 0)
	{
		matrix = new int[rows * columns];
		srand(time(NULL));
		for (int i = 0; i < rows * columns; i++) // ���������� �������
			matrix[i] = rand() % 100;        // ���������� �� 0 �� 100

		if ((rows < 10) && (columns < 10)) // ���� ������� ������ ��� 10 �� 10 �� ������� �� �� �����
			OutputMatr(matrix, rows, columns);
	}
	if (proc_rank == 0)
	{
		end = MPI_Wtime();

		cout << "max = " << GlobMax << endl;
		cout << "Parallel time : " << end - start << endl;

		// �������� �����
		start = MPI_Wtime();
		max = MaxSearch(matrix, rows * columns); // ��������� ��������
		end = MPI_Wtime();
		cout << fixed << "Linear time   : " << end - start << endl;

		if (GlobMax == max) // �������� �� �� ��� �������� � ������������ ���������� ��������� ���������
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