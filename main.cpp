/* 
 * Author: Guojun Chen, gc34@rice.edu
 * Main function
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <ctime>

using namespace std;

const char usage[] = {
"Command Syntax:\n\
\t./CNF filename\n\n\
Required arguments:\n\
\tfilename can be absolute or relative path to the input file\n\n"
};



#define MAX_PROP 201	// maximum number of propositions
#define MAX_CLAU 1501	// maximum number of clauses

bool randomSolver(int* input, int np, int nc);

int *input_buffer;
int numberOfPropositionPerClause[MAX_CLAU];
int lastPropositionPerClause[MAX_CLAU];
bool result_buffer[MAX_PROP];
int prop, clause;

int main(int argc, char const *argv[]) {

	if (argc < 2) {
		fprintf(stderr, "Error: please input filename.\n");
		printf("%s\n", usage);
		return -1;
	}

	FILE* stream;
	if ((stream = fopen(argv[1], "r")) == NULL) {
		fprintf(stderr, "Error: bad filename.\n");
		return -1;
	}

	ssize_t nread;
	size_t input_len = 100;
	char* input_buf;
	input_buf = (char *)malloc(input_len * sizeof(char));

	while ((nread = getline(&input_buf, &input_len, stream)) != -1) {
		if (input_buf[0] == 'c') continue;
		else if (input_buf[0] == 'p') {
			sscanf(input_buf, "p cnf %d %d", &prop, &clause);
			if (prop <= 0 || clause <= 0) {
				fprintf(stderr, "Invalid value for propositions or clauses.\n");
			}
			break;
		}
	}

	input_buffer = new int[prop * clause];
	int valprop, cindex;
	cindex = 0;
	while (fscanf(stream, "%d", &valprop) > 0) {
		if (valprop == 0) {
			cindex++;
		} else {
			input_buffer[cindex * prop + abs(valprop) - 1] = valprop / abs(valprop);
			numberOfPropositionPerClause[cindex]++;
			lastPropositionPerClause[cindex] = abs(valprop) - 1;
		}
	}


	bool *active_clause = new bool[MAX_CLAU];
	memset(active_clause, 1, clause);
	for (int i = 0; i < clause; i++) {
		if (numberOfPropositionPerClause[i] == 1) {
			int p = lastPropositionPerClause[i];
			result_buffer[p] = input_buffer[i * prop + p] > 0;
			for (int j = 0; j < clause; j++) {
				if (result_buffer[p]) {
					if (input_buffer[j * prop + p] > 0)
						active_clause[j] = 0;
					else if (input_buffer[j * prop + p] < 0)
						input_buffer[j * prop + p] = 0;
				} else {
					if (input_buffer[j * prop + p] < 0)
						active_clause[j] = 0;
					else if (input_buffer[j * prop + p] > 0)
						input_buffer[j * prop + p] = 0;
				}
				
			}

		}
	}
	int activeCnt = 0;
	for (int i = 0; i < clause; i++) {
		if (active_clause[i]) {
			memcpy(input_buffer + activeCnt * prop, input_buffer + i * prop, prop * sizeof(int));
			activeCnt++;
		}
	}
	delete active_clause;
	clause = activeCnt;

#if 0
	cout << "Number of propositions: " << prop << endl;
	cout << "Number of clauses: " << clause << endl;
	for (int i = 0; i < clause; i++) {
		for (int j = 0; j < prop; j++)
			cout << input_buffer[i * prop + j] << ", ";
		cout << endl;
	}
#endif

	time_t now = time(0);
	cout << "Begin time: " << now << endl;

	if (randomSolver(input_buffer, prop, clause)) {
		printf("Satifiable.\n");
		for (int i = 0; i < prop; i++)
			cout << result_buffer[i] << ", ";
		cout << endl;
	}
	else printf("Not satisfiable.\n");

	now = time(0);
	cout << "End time: " << now << endl;

	delete input_buffer;
	return 0;
}

#define DEBU

#ifdef DEBUG
	#define debugOut cout
#else
	class NullBuffer : public std::streambuf {
	public:
	  int overflow(int c) { return c; }
	};
	NullBuffer null_buffer;
	std::ostream null_stream(&null_buffer);
	#define debugOut null_stream
#endif

bool randomSolver(int* input, int np, int nc) {
	if (nc == 1) {
		for (int i = 0; i < np; i++) {
			if (input[i]) {
				result_buffer[i] = input[i] > 0;
				return true;
			}
		}
		return false;
	}

	int *local_buffer = new int[np * nc];
	bool *active_clause = new bool[nc];

	int cp = rand() % np + 1;
	while (input[cp] == 0) cp = (cp + 1) % np + 1;
	debugOut << "Chose " << cp << endl;

	// assign it to true
	result_buffer[cp] = true;
	memset(active_clause, 1, nc);
	memcpy(local_buffer, input, np*nc*sizeof(int));
	
	for (int i = 0; i < nc; i++) {
		if (local_buffer[i*np + cp] > 0) active_clause[i] = 0;
		else if (local_buffer[i*np + cp] < 0) local_buffer[i*np + cp] = 0;
	}

	int activeCnt = 0;
	for (int i = 0; i < nc; i++) {
		if (active_clause[i]) {
			memcpy(local_buffer + activeCnt * np, local_buffer + i * np, np*sizeof(int));
			activeCnt++;
		}
	}

	debugOut << "############TRUE##############" << endl;
	debugOut << "Number of propositions: " << np << endl;
	debugOut << "Number of clauses: " << activeCnt << endl;
	for (int i = 0; i < activeCnt; i++) {
		for (int j = 0; j < np; j++)
			debugOut << local_buffer[i * np + j] << ", ";
		debugOut << endl;
	}

	bool sr = randomSolver(local_buffer, np, activeCnt);
	if (sr) {
		delete active_clause;
		delete local_buffer;
		return true;
	}

	// assign it to false
	result_buffer[cp] = false;
	memset(active_clause, 1, nc);
	memcpy(local_buffer, input, np*nc*sizeof(int));
	
	for (int i = 0; i < nc; i++) {
		if (local_buffer[i*np + cp] < 0) active_clause[i] = 0;
		else if (local_buffer[i*np + cp] > 0) local_buffer[i*np + cp] = 0;
	}

	activeCnt = 0;
	for (int i = 0; i < nc; i++) {
		if (active_clause[i]) {
			memcpy(local_buffer + activeCnt * np, local_buffer + i * np, np*sizeof(int));
			activeCnt++;
		}
	}

	debugOut << "############FALSE##############" << endl;
	debugOut << "Number of propositions: " << np << endl;
	debugOut << "Number of clauses: " << activeCnt << endl;
	for (int i = 0; i < activeCnt; i++) {
		for (int j = 0; j < np; j++)
			debugOut << local_buffer[i * np + j] << ", ";
		debugOut << endl;
	}

	bool fr = randomSolver(local_buffer, np, activeCnt);
	if (fr) {
		delete active_clause;
		delete local_buffer;
		return true;
	}
	
	delete active_clause;
	delete local_buffer;
	return false;
}