/* 
 * Author: Guojun Chen, gc34@rice.edu
 * Main function
 */

#include "cnf.h"
#include "debug.h"
using namespace std;

#ifndef DEBUG
extern NullBuffer nullbuffer;
extern std::ostream null_stream;
#endif

const char usage[] = {
"Command Syntax:\n\
\t./CNF filename\n\n\
Required arguments:\n\
\tfilename can be absolute or relative path to the input file\n\n"
};

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

	CNF instance(stream);
	if (instance.satisfiable()) {
		cout << "SAT" << endl;
		int sum = 0;
		int set = 0;
		for (int i = 1; i <= instance.numOfVars(); i++) {
			cout << "p" << i << ": " << !instance.propGetVar(i).sign << endl;
			sum += instance.propIsFalse(i);
			set += instance.propGetVar(i).set;
		}
		debugOut << set << endl;
		debugOut << sum << endl;

	} else {
		cout << "UNSAT" << endl;
	}

	return 0;
}
