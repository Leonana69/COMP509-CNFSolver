#include <iostream>
#include <cmath>
#include <stack>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
using namespace std;

void perLine(int a, int b, int c, int d, int e) {
	printf("%d %d %d %d %d 0\n", a, b, c, d, e);
	// printf("%d 	%d %d %d -%d 0\n", a, b, c, d, e);
	// printf("%d 	%d %d -%d %d 0\n", a, b, c, d, e);
	printf("%d %d %d -%d -%d 0\n", a, b, c, d, e);
	// printf("%d 	%d -%d %d %d 0\n", a, b, c, d, e);
	printf("%d %d -%d %d -%d 0\n", a, b, c, d, e);
	printf("%d %d -%d -%d %d 0\n", a, b, c, d, e);
	printf("%d %d -%d -%d -%d 0\n", a, b, c, d, e);
	// printf("%d -%d %d %d %d 0\n", a, b, c, d, e);
	printf("%d -%d %d %d -%d 0\n", a, b, c, d, e);
	printf("%d -%d %d -%d %d 0\n", a, b, c, d, e);
	printf("%d -%d %d -%d -%d 0\n", a, b, c, d, e);
	printf("%d -%d -%d %d %d 0\n", a, b, c, d, e);
	printf("%d -%d -%d %d -%d 0\n", a, b, c, d, e);
	printf("%d -%d -%d -%d %d 0\n", a, b, c, d, e);
	printf("%d -%d -%d -%d -%d 0\n", a, b, c, d, e);
	// printf("-%d %d %d %d %d 0\n", a, b, c, d, e);
	printf("-%d %d %d %d -%d 0\n", a, b, c, d, e);
	printf("-%d %d %d -%d %d 0\n", a, b, c, d, e);
	printf("-%d %d %d -%d -%d 0\n", a, b, c, d, e);
	printf("-%d %d -%d %d %d 0\n", a, b, c, d, e);
	printf("-%d %d -%d %d -%d 0\n", a, b, c, d, e);
	printf("-%d %d -%d -%d %d 0\n", a, b, c, d, e);
	printf("-%d %d -%d -%d -%d 0\n", a, b, c, d, e);
	printf("-%d -%d %d %d %d 0\n", a, b, c, d, e);
	printf("-%d -%d %d %d -%d 0\n", a, b, c, d, e);
	printf("-%d -%d %d -%d %d 0\n", a, b, c, d, e);
	printf("-%d -%d %d -%d -%d 0\n", a, b, c, d, e);
	printf("-%d -%d -%d %d %d 0\n", a, b, c, d, e);
	printf("-%d -%d -%d %d -%d 0\n", a, b, c, d, e);
	printf("-%d -%d -%d -%d %d 0\n", a, b, c, d, e);
	printf("-%d -%d -%d -%d -%d 0\n", a, b, c, d, e);
}

void generatorEinstein() {
	for (int i = 0; i < 25; i++) {
		int x = 5 * i;
		perLine(x+1, x+2, x+3, x+4, x+5);
	}

	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) {
			int a = i * 25 + j + 1;
			perLine(a, a+5, a+10, a+15, a+20);
		}
	}
}

void randomCNF(int N, int L) {
	srand(time(0));
	ofstream output;
	output.open("./data/N" + to_string(N) + "L" + to_string(L) + ".cnf", ios::trunc);
	output << "p cnf " << N << " " << L << endl;
	for (int i = 0; i < L; i++) {
		int a = rand() % N + 1;
		int b = rand() % N + 1;
		int c = rand() % N + 1;
		int sa = (((float)rand() / RAND_MAX) > 0.5 ? -1 : 1);
		int sb = (((float)rand() / RAND_MAX) > 0.5 ? -1 : 1);
		int sc = (((float)rand() / RAND_MAX) > 0.5 ? -1 : 1);
		output << sa * a << " ";
		output << sb * b << " ";
		output << sc * c << " ";
		output << 0 << endl;
	}
}

int main(int argc, char const *argv[]) {
	int n = atoi(argv[1]);
	for (int i = 0; i < 16; i++) {
		randomCNF(n, (float)n * (i*0.2 + 3.0));
	}
}