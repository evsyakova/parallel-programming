#include <iostream>
#include "MPI.h"


int MaxSearch(const int* vector, const int n)
{
	int max;
	if (n == 0)
		return INT_MAX;

	max = vector[0];
	for (int i = 0; i < n; i++)
		if (max < vector[i])
			max = vector[i];

	return max;
}

void OutputMatr(const int* matrix, const int n, const int m)
{
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < m; j++)
			printf("%d ", matrix[i * m + j]);
		printf("\n");
	}
}

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		printf("Usage: %s, n, m \n", argv[0]);
		printf("n - rows\n");
		printf("m - columns\n");
		return 1;
	}

	int* matrix = 0;
	int n, m;
	int max;


	int proc_num, proc_rank;
	int *send_counts, *displs, *recieve_buffer;
	int chunk_size, rem;

	double start, end;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &proc_num);
	MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);

	n = atoi(argv[1]);
	m = atoi(argv[2]);
	if (proc_rank == 0)
	{
		matrix = new int[n * m];
		srand(5);
		for (int i = 0; i < n * m; i++)
			matrix[i] = rand();
		if ((n < 10) && (m < 10))
			OutputMatr(matrix, n, m);
	}

	start = MPI_Wtime();
	send_counts = new int[proc_num];
	displs = new int[proc_num];
	chunk_size = n * m / proc_num;
	rem = n * m % proc_num;

	send_counts[0] = chunk_size + rem;
	displs[0] = chunk_size + rem;
	for (int i = 0; i < proc_num; i++)
	{
		send_counts[i] = chunk_size;
		displs[i] = rem + i * chunk_size;
	}

	recieve_buffer = new int[send_counts[proc_rank]];
	MPI_Scatterv(matrix, send_counts, displs, MPI_INT, recieve_buffer, send_counts[proc_rank], MPI_INT, 0, MPI_COMM_WORLD);

	max = MaxSearch(recieve_buffer, send_counts[proc_rank]);
	int GlobMax = 0;

	MPI_Reduce(&max, &GlobMax, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

	if (proc_rank == 0)
	{
		end = MPI_Wtime();

		printf("max= %d\n", GlobMax);
		printf("Time spent: %lf\n", end - start);

		delete matrix;
	}
	MPI_Finalize();

	delete[] send_counts;
	delete[] displs;
	delete[] recieve_buffer;

	return 0;
}