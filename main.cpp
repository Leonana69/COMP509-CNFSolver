/* 
 * Author: Guojun Chen, gc34@rice.edu
 * Main function
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

using namespace std;

const char usage[] = {
"Command Syntax:\n\
\t./412alloc [flag] filename\n\n\
Required arguments:\n\
\tfilename can be absolute or relative path to the input file\n\n"
};

bool randomSolver();

#define MAX_PROP 201
#define MAX_CLAU 500
int input_CNF_buffer[MAX_CLAU][MAX_PROP];
int randomSolver_buffer[MAX_CLAU][MAX_PROP];
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

	int valprop, cindex;
	cindex = 0;
	while (fscanf(stream, "%d", &valprop) > 0) {
		if (valprop == 0) {
			cindex++;
		} else {
			input_CNF_buffer[cindex][abs(valprop)] = valprop / abs(valprop);
		}
	}

#ifdef INPUT_DEBUG
	for (int i = 0; i < clause; i++) {
		for (int j = 0; j < prop + 1; j++)
			cout << input_CNF_buffer[i][j] << ", ";
		cout << endl;
	}
#endif
	if (randomSolver()) printf("Satifiable.\n");
	else printf("Not satisfiable.\n");

	return 0;
}

bool randomSolver() {
	memcpy(randomSolver_buffer, input_CNF_buffer, sizeof(input_CNF_buffer));
	
}