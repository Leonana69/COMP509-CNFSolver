/* 
 * Author: Guojun Chen, gc34@rice.edu
 * Main function
 */

#include "cnf.h"
#include "debug.h"
#include <chrono>
#include <algorithm>
#include <iostream>

using namespace std;

#ifndef DEBUG
extern NullBuffer nullbuffer;
extern std::ostream null_stream;
#endif

const char usage[] = {
"Command Syntax:\n\
\t./CNF [-flags] filename/N\n\n\
Required arguments:\n\
\tfilename can be absolute or relative path to the input file\n\n\
Optional flags:\n\
\t-s\toutput the assignment (if it's satisfiable), this can not be used with -g\n\
\t-i [F]\tsolve the input file F, this can not be used with -g\n\
\t-g [N]\tgenerate test, input a number of propositions N\n\
\t-r\tdo not run the random solution\n"
};

void loadCNF(FILE* stream, int& num_props, vector<vector<int>>& clauses);
void randomCNF(int N, int L, vector<vector<int>>& clauses);
void test(int N, vector<vector<int>>& clauses);

bool run_random = true;

int main(int argc, char const *argv[]) {
	int flag_inputFile;
	int index_inputFile;
	
	bool show_res = false;
	int num_props;
	vector<vector<int>> clauses;
	auto start = chrono::high_resolution_clock::now();
	auto elapsed = chrono::high_resolution_clock::now() - start;

	if (argc < 3) {
		fprintf(stderr, "Error: please input filename or flags.\n");
		printf("%s\n", usage);
		return -1;
	}

	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
				case 'g':
					num_props = atoi(argv[i + 1]);
					i++;
					flag_inputFile = 0;
					break;
				case 'i':
					flag_inputFile = 1;
					index_inputFile = i + 1;
					i++;
					break;
				case 's':
					show_res = true;
					break;
				case 'r':
					run_random = false;
					break;
				default:
					fprintf(stderr, "Error: unknown flags.\n");
					return -1;
			}
		}
	}

	

	if (flag_inputFile) {
		FILE* stream;
		if ((stream = fopen(argv[index_inputFile], "r")) == NULL) {
			fprintf(stderr, "Error: bad filename.\n");
			return -1;
		}
		loadCNF(stream, num_props, clauses);
	} else {
		
		test(num_props, clauses);
		return 0;
	}

	start = chrono::high_resolution_clock::now();
	CNF instance(num_props, clauses, 0);
	elapsed = chrono::high_resolution_clock::now() - start;
	float microseconds = chrono::duration_cast<std::chrono::microseconds>(elapsed).count() / 1000000.0;
	cout << "Time: " << microseconds << endl;
	cout << "Calls: " << instance.numberOfCalls() << endl;
	if (instance.satisfiable()) {
		cout << "SAT" << endl;
		// int sum = 0;
		// int set = 0;
		if (show_res)
			for (int i = 1; i <= instance.numOfVars(); i++) {
				cout << "p" << i << ": " << !instance.propGetVar(i).sign << endl;
				// sum += instance.propIsFalse(i);
				// set += instance.propGetVar(i).set;
			}
		// debugOut << set << endl;
		// debugOut << sum << endl;
	} else {
		cout << "UNSAT" << endl;
	}
	return 0;
}

void test(int N, vector<vector<int>>& clauses) {
	int result[16];
	double times[3][16][100];
	int calls[3][16][100];
	auto start = chrono::high_resolution_clock::now();
	auto elapsed = chrono::high_resolution_clock::now() - start;
	srand(time(0)); // there can only be one srand
	memset(result, 0, sizeof(result));
	memset(times, 0, sizeof(times));
	memset(calls, 0, sizeof(calls));
	CNF* instance;
	for (int j = 0; j < 16; j++) {
		cout << " j = " << j << "\n";
		int L = (3 + j * 0.2) * N;

		for (int i = 0; i < 100; i++) {
			randomCNF(N, L, clauses); // generate once for the following three methods

			if (run_random) {
				start = chrono::high_resolution_clock::now();
				instance = new CNF(N, clauses, 0);
				elapsed = chrono::high_resolution_clock::now() - start;
				times[0][j][i] = chrono::duration_cast<std::chrono::microseconds>(elapsed).count() / 1000000.0;
				calls[0][j][i] = instance->numberOfCalls();
				delete instance;
			}

			start = chrono::high_resolution_clock::now();
			instance = new CNF(N, clauses, 1);
			elapsed = chrono::high_resolution_clock::now() - start;
			times[1][j][i] = chrono::duration_cast<std::chrono::microseconds>(elapsed).count() / 1000000.0;
			calls[1][j][i] = instance->numberOfCalls();
			result[j] += instance->satisfiable();
			delete instance;

			start = chrono::high_resolution_clock::now();
			instance = new CNF(N, clauses, 2);
			elapsed = chrono::high_resolution_clock::now() - start;
			times[2][j][i] = chrono::duration_cast<std::chrono::microseconds>(elapsed).count() / 1000000.0;
			calls[2][j][i] = instance->numberOfCalls();

			delete instance;
		}
		
	}
	if (run_random) {
		cout << "Random selector:" << endl;
		for (int j = 0; j < 16; j++) {
			int L = (3 + j * 0.2) * N;
			sort(times[0][j], times[0][j] + 100);
			sort(calls[0][j], calls[0][j] + 100);
			printf("L/N = %d/%d, mtime: %.6lf, mcalls: %3d, sat probability: %.3lf\n", L, N, times[0][j][50], calls[0][j][50], (float)result[j] / 100.0);
		}
	}
	
	cout << "Two clauses selector:" << endl;

	for (int j = 0; j < 16; j++) {
		int L = (3 + j * 0.2) * N;
		sort(times[1][j], times[1][j] + 100);
		sort(calls[1][j], calls[1][j] + 100);
		printf("L/N = %d/%d, mtime: %.6lf, mcalls: %3d, sat probability: %.3lf\n", L, N, times[1][j][50], calls[1][j][50], (float)result[j] / 100.0);
	}

	cout << "Two clauses with pos selector:" << endl;

	for (int j = 0; j < 16; j++) {
		int L = (3 + j * 0.2) * N;
		sort(times[2][j], times[2][j] + 100);
		sort(calls[2][j], calls[2][j] + 100);
		printf("L/N = %d/%d, mtime: %.6lf, mcalls: %3d, sat probability: %.3lf\n", L, N, times[2][j][50], calls[2][j][50], (float)result[j] / 100.0);
	}
}

// generate random 3-CNF formula
void randomCNF(int N, int L, vector<vector<int>>& clauses) {
	clauses.clear();
	vector<int> vtmp;
	for (int i = 0; i < L; i++) {
		vtmp.clear();
		int a = rand() % N + 1;
		int b = rand() % N + 1;
		while (b == a) b = rand() % N + 1;
		int c = rand() % N + 1;
		while (c == b || c == a) c = rand() % N + 1;
		int sa = (((float)rand() / RAND_MAX) > 0.5 ? -1 : 1);
		int sb = (((float)rand() / RAND_MAX) > 0.5 ? -1 : 1);
		int sc = (((float)rand() / RAND_MAX) > 0.5 ? -1 : 1);
		vtmp.push_back(sa * a);
		vtmp.push_back(sb * b);
		vtmp.push_back(sc * c);
		clauses.push_back(vtmp);
	}
}

// load the input
void loadCNF(FILE* stream, int& num_props, vector<vector<int>>& clauses) {
	int num_clauses;
	clauses.clear();
	size_t input_len = 100;
	char* input_buf;
	input_buf = (char *)malloc(input_len * sizeof(char));

	while (getline(&input_buf, &input_len, stream) != -1) {
		if (input_buf[0] == 'c') continue;
		else if (input_buf[0] == 'p') {
			sscanf(input_buf, "p cnf %d %d", &num_props, &num_clauses);
			if (num_props <= 0 || num_clauses <= 0) {
				fprintf(stderr, "Invalid value for propositions or clauses.\n");
			}
			break;
		}
	}

	int *cnf_buffer = new int[num_props * num_clauses];

	vector<int>clause;

	int valprop;
	while (fscanf(stream, "%d", &valprop) > 0) {
		if (valprop == 0) {
			clauses.push_back(clause);
			clause.clear();
		} else clause.push_back(valprop);
	}

	debugOut << "Number of propositions: " << num_props << endl;
	debugOut << "Number of clauses: " << clauses.size() << endl;

	for (int i = 0; i < clauses.size(); i++) {
		auto v = clauses[i];
		debugOut << "Clause " << i + 1 << ":";
		for (int j = 0; j < v.size(); j++) {
			debugOut << " " << v[j];
		}
		debugOut << endl;
	}
}
