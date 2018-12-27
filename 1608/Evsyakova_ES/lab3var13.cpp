// mpilab.cpp: определяет точку входа для консольного приложения.
// By Nakhimovich Dm. 0836-1
//

#include "stdafx.h"
#include "mpi.h"
#include <clocale>
#include <Windows.h>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <random>
#include <ctime>
#include <string>
#include <cstdlib>
#include <conio.h>
using namespace std;

typedef unsigned int uint;

//+ REHASH FUNC FOR RABIN-KARP
uint REHASH(const uint old_first, const uint new_last, const uint old_hash, const uint q, const uint d) {
	return ((((old_hash)-((old_first)*d)) << 1) + (new_last)) % q;
}
//+ RABIN-KARP SEARCH dynamic hash
void KR(char *sub, uint sub_size, char *str, uint str_size, int res[]) {
	uint hsub, hstr, i, j;
	unsigned int d;
	uint entry = 0;
	uint comparison = 0;
	bool flag = false;

	//Here all the modulo multiplications by 2 are made using shifts.	

	uint q = (uint)4294967291;//2305843009213693951; // very big prime number

	/* Preprocessing */

	// computes d = 2^(m-1) with the left-shift operator 
	for (d = i = 1; i < sub_size; ++i)
		d = (d << 1);

	for (hsub = hstr = i = 0; i < sub_size; ++i) {// via Gorner multiplications
		hsub = ((hsub << 1) + (int)sub[i] + 200) % q;
		hstr = ((hstr << 1) + (int)str[i] + 200) % q;
	}

	/* Searching */
	j = 0;
	while (j <= str_size - sub_size) {
		comparison++;
		if (hsub == hstr) {
			for (unsigned int i = 0; i < sub_size; i++) {
				comparison++;
				if (sub[i] != str[j + i]) {
					flag = true;
					break;
				}
			}

			if (flag == false) {
				entry++;
				res[j] = j;
				//cout << (j) << " " << endl;				
			}
			else
				res[j] = -1;
			flag = false;
		}
		else
			res[j] = -1;
		hstr = REHASH(((int)str[j] + 200), ((int)str[j + sub_size] + 200), hstr, q, d);
		j++;
	}

	//cout << "number of  comparison opertions:" << comparison << endl;
	//cout << "number of substring entry: " << entry << endl;
}


int main(int argc, char* argv[])
{
	// console 
	setlocale(LC_CTYPE, "rus");
	SetConsoleCP(1251);// установка кодовой страницы win-cp 1251 в поток ввода
	SetConsoleOutputCP(1251); // установка кодовой страницы win-cp 1251 в поток вывода
	srand((unsigned)time(NULL));
	// Char arrays
	char *text = 0, *sub = 0;
	string substring;
	uint text_size = 0, sub_size = 0;
	ifstream file;
	// Result 
	int* result_pos = 0;
	// Time
	double time_linear_start, time_linear_end, time_parallel_start, time_parallel_end;
	// MPI info
	int proc_rank, proc_size;
	// Send param
	int *send_counts, *send_displs;
	char *scatter_buffer;
	// Recive param
	int *rec_counts, *rec_displs, *gather_buffer;
	// org values
	uint chunk, rem, *row_num;

	// ======================  Start Parallel Part ======================
	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &proc_size);
	// chek porc count
	if (proc_size < 1) {
		cout << "wrong proc number (put -n >0) " << endl;
		return 0;
	}

	// Root Proc gen text and sub from user request
	if (proc_rank == 0) {
		int type = 0;
		cout << endl;
		cout << " enter text type: 1. open file 2. create text " << endl;
		cin >> type;
		if (type == 1) {
			string text_name = "test.txt"; char word; uint count = 0;

			cout << "Put the way to file or file name: " << endl;
			cin >> text_name;

			file.open(text_name.c_str(), ios::in);
			if (file) {
				cout << "file opened..." << endl;

				while (!file.eof()) {
					file.get(word);
					count++;
				}
				text_size = count;
				file.close();
			}
			else {
				cout << "error! input file incorrect" << endl;
				text_name = "not found";
				text_size = count;
				MPI_Finalize();
				return 0;
			}
			// create text 
			uint i = 0;
			file.open(text_name.c_str(), ios::in);
			if (file) {

				if (text != NULL)
					delete[] text;

				text = new char[text_size];
				while (!file.eof()) {
					file >> text[i];
					//cout << text_c[i];
					i++;
				}
				file.close();
			}
			else {
				cout << "error to open text!" << endl;
				text_name = "not found";
				text_size = 0;
				MPI_Finalize();
				return 0;
			}
		}
		if (type == 2) {
			cout << "enter the word which will be composed text: " << endl;

			string word;
			cin >> word;

			cout << "enter number of copy: " << endl;

			uint size;
			cin >> size;

			if (size <= 0) {
				cout << " error to gen text!" << endl;
				MPI_Finalize();
				return 0;
			}

			if (text != NULL)
				delete[] text;
			text = new char[size * word.length()];

			for (uint i = 0; i < size; ++i) {
				for (uint j = 0; j < word.length(); ++j) {
					text[word.length()*i + j] = (char)word[j];
					//cout << text_c[i + j];
				}
			}
			
			text_size = size * word.length();
		}
		if (type != 1 && type != 2) {
			cout << "enter correct type!" << endl;
			MPI_Finalize();
			return 0;
		}
		cout << " enter sub type: 1. enter word 2. create word " << endl;
		cin >> type;
		if (type == 1) {
			cout << "put some substring to serch it in text: " << endl;
			cin >> substring;
			//cout << substring << endl;

			if (sub != NULL)
				delete[] sub;
			sub = new char[substring.length()];

			for (uint j = 0; j < substring.length(); ++j) {
				sub[j] = substring[j];
				//cout << sub_c[j];
			}
			sub_size = substring.length();
		}
		if (type == 2) {
			cout << "enter the word which will be composed substring" << endl;

			string word;
			cin >> word;

			cout << "enter number of copy: " << endl;

			uint size;
			cin >> size;

			if (size <= 0) {
				cout << " error to gen sub!" << endl;
				MPI_Finalize();
				return 0;
			}

			substring.clear();
			if (sub != NULL)
				delete[] sub;
			sub = new char[size * word.length()];

			for (uint i = 0; i < size; ++i) {
				for (uint j = 0; j < word.length(); ++j) {
					sub[word.length() * i + j] = (char)word[j];
				}
			}

			sub_size = size*word.length();
		}
		if (type != 1 && type != 2) {
			cout << "enter correct type!" << endl;
			MPI_Finalize();
			return 0;
		}
		result_pos = new int[text_size - sub_size + 1];
		
		if (!text || !sub || !result_pos) {
			cout << " error to gen arrays! " << endl;
			MPI_Finalize();
			return 0;
		}
		
		// Create block parametrs for each proc
		send_counts = new int[proc_size];
		send_displs = new int[proc_size];
		rec_counts = new int[proc_size];
		rec_displs = new int[proc_size];
		//row_num = new int[proc_size];
		chunk = text_size / proc_size;
		rem = text_size % proc_size;
		for (int i = 0; i < proc_size - 1; i++)
		{
			send_counts[i] = chunk + sub_size - 1;
			send_displs[i] = i * (chunk);
			rec_counts[i] = chunk;
			rec_displs[i] = i * (chunk);
		}		
		send_counts[proc_size - 1] = chunk + rem;
		send_displs[proc_size - 1] = ((proc_size - 1) * chunk);
		rec_counts[proc_size - 1] = chunk + rem - sub_size + 1;
		rec_displs[proc_size - 1] = (proc_size - 1) * (chunk);

	}	
	else
	{
		send_counts = new int[proc_size];
		send_displs = new int[proc_size];
		rec_counts = new int[proc_size];
		rec_displs = new int[proc_size];
		//row_num = new int[proc_size];
	}
	
	//Bcast values for all proc
	if (proc_rank != 0) {
		sub = new char[sub_size];
	}
	MPI_Bcast(&sub_size, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
	MPI_Bcast(sub, sub_size, MPI_CHAR, 0, MPI_COMM_WORLD);
	MPI_Bcast(send_counts, proc_size, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(send_displs, proc_size, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(rec_counts, proc_size, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(rec_displs, proc_size, MPI_INT, 0, MPI_COMM_WORLD);
	//MPI_Bcast(row_num, proc_size, MPI_INT, 0, MPI_COMM_WORLD);

	/*	
	// block test
	if (proc_rank == 0) {
		cout << " _______ info in proc " << proc_rank << " _______" << endl;
		cout << "send_counts: "; show_vector(send_counts, proc_size);
		cout << "send_displs: "; show_vector(send_displs, proc_size);
		cout << "rec_counts: "; show_vector(rec_counts, proc_size);
		cout << "rec_displs: "; show_vector(rec_displs, proc_size);
		cout << "row_num: "; show_vector(row_num, proc_size);
	}*/
			
	// Create container
	scatter_buffer = new char[send_counts[proc_rank]];	
	gather_buffer = new int[rec_counts[proc_rank]];

	// Main Parallel algoritm 
	if (proc_rank == 0)
		time_parallel_start = MPI_Wtime();
	// Send parts to proc		
	MPI_Scatterv(text, send_counts, send_displs, MPI_CHAR, scatter_buffer, send_counts[proc_rank], MPI_CHAR, 0, MPI_COMM_WORLD);
	
	// Send test
	//cout << " ______ send proc: " << proc_rank << " ________ "<< endl;
	//show_matrix_vector(scatter_buffer, matrix1_col, row_num[proc_rank]); 

	// Main Calc
	KR(sub, sub_size, scatter_buffer, send_counts[proc_rank], gather_buffer);

	// Gather test
	//cout << " ______  gather proc: " << proc_rank << " ________ " << endl;
	//show_matrix_vector(gather_buffer, matrix2_col, row_num[proc_rank]);

	// Gather all parts to root proc	
	MPI_Gatherv(gather_buffer, rec_counts[proc_rank], MPI_INT, result_pos, rec_counts, rec_displs, MPI_INT, 0, MPI_COMM_WORLD);

	// Show results
	if (proc_rank == 0)
	{
		// End of the parallel algorythm
		time_parallel_end = MPI_Wtime();

		// ====================== Parallel Results ============================

		cout <<endl << " ====== PARALLEL VERSION ====== " << endl;	
		
		//for (uint i = 0; i < (text_size - sub_size + 1); ++i)
		//	cout << result_pos[i] << " ";
		
		cout << " Parallel alg ended with time: " << time_parallel_end - time_parallel_start << " sec" << endl;

		// ====================== Linear Program ==============================
		int *res = new int[text_size - sub_size + 1];
		time_linear_start = MPI_Wtime();
		KR(sub, sub_size, text, text_size, res);
		time_linear_end = MPI_Wtime();

		// ====================== Linear Results ============================
		
		cout << endl << " ====== LINEAR VERSION ====== " << endl;

		//for (uint i = 0; i < (text_size - sub_size + 1); ++i)
		//	cout << res[i] << " ";

		cout << " Linear alg ended with time: " << time_linear_end - time_linear_start << " sec" << endl;
		
		// Final Result
		cout << " Boost in sec: " << ((time_linear_end - time_linear_start) - (time_parallel_end - time_parallel_start)) << endl;
		cout << " Boost in eq: " << ((time_linear_end - time_linear_start) / (time_parallel_end - time_parallel_start)) << endl;

		// Clear space created
		delete[] res;		

	}
	
	// Clear space created
	delete[] send_counts;
	delete[] rec_counts;
	delete[] rec_displs;
	delete[] send_displs;
	//delete[] row_num;
	delete[] gather_buffer;
	delete[] scatter_buffer;
	
	// =========================== End Parallel Part ===========================
	MPI_Finalize();		
	
	//_getch();	


    return 0;
}

