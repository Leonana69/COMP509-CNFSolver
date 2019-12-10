/* 
 * Author: Guojun Chen, gc34@rice.edu
 * Main function
 */

#include "cnf.h"
#include "debug.h"
#include <chrono>

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
\t-i\tsolve the input file\n\
\t-g\tgenerate test, input a number of propositions N\n"
};

void loadCNF(FILE* stream, int& num_props, vector<vector<int>>& clauses);
void randomCNF(int N, int L, vector<vector<int>>& clauses);

int main(int argc, char const *argv[]) {
	int flag_inputFile;
	int num_props;
	vector<vector<int>> clauses;
	auto start = chrono::high_resolution_clock::now();
	auto elapsed = chrono::high_resolution_clock::now() - start;

	if (argc < 3) {
		fprintf(stderr, "Error: please input filename or flags.\n");
		printf("%s\n", usage);
		return -1;
	}

	if (argv[1][0] == '-') {
		switch (argv[1][1]) {
			case 'g':
				num_props = atoi(argv[2]);
				flag_inputFile = 0;
				break;
			case 'i':
				flag_inputFile = 1;
				break;
			default:
				fprintf(stderr, "Error: unknown flags.\n");
				return -1;
		}
	}

	if (flag_inputFile) {
		FILE* stream;
		if ((stream = fopen(argv[2], "r")) == NULL) {
			fprintf(stderr, "Error: bad filename.\n");
			return -1;
		}
		loadCNF(stream, num_props, clauses);
	} else {
		randomCNF(num_props, 3 * num_props, clauses);
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
		// for (int i = 1; i <= instance.numOfVars(); i++) {
		// 	cout << "p" << i << ": " << !instance.propGetVar(i).sign << endl;
		// 	sum += instance.propIsFalse(i);
		// 	set += instance.propGetVar(i).set;
		// }
		// debugOut << set << endl;
		// debugOut << sum << endl;
	} else {
		cout << "UNSAT" << endl;
	}
	return 0;
}

// generate random 3-CNF formula
void randomCNF(int N, int L, vector<vector<int>>& clauses) {
	srand(time(0));
	vector<int> vtmp;
	for (int i = 0; i < L; i++) {
		vtmp.clear();
		int a = rand() % N + 1;
		int b = rand() % N + 1;
		int c = rand() % N + 1;
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
