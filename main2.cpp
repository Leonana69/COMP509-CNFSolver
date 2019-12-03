/* 
 * Author: Guojun Chen, gc34@rice.edu
 * Main function
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <ctime>
#include <vector>

using namespace std;

#define DEBUG

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

const char usage[] = {
"Command Syntax:\n\
\t./CNF filename\n\n\
Required arguments:\n\
\tfilename can be absolute or relative path to the input file\n\n"
};

struct Variable {
	bool set;
	bool sign;
	bool mark;
	bool unit;
	bool unit_sign;
	int dlevel;
	vector<int>* reason;
	vector<vector<int>> watches[2];
	void setUnit(bool _sign) {
		unit = true;
		unit_sign = _sign;
	}
	Variable() {
		set = false;
		sign = false;
		mark = false;
		unit = false;
		unit_sign = false;
		dlevel = 0;
		reason = NULL;
		watches[0].clear();
		watches[1].clear();
	}
};

struct State {
	bool empty;
	vector<Variable> vars;
	vector<int> clauses;
	vector<int> trail;
	int dlevel;
	int tlevel;
	State() {
		empty = false;
		dlevel = tlevel = 0;
		vars.clear();
		clauses.clear();
		trail.clear();
	}
};

inline int literalGetIdx(int literal) { return literal < 0 ? -literal : literal; }
inline int literalGetSign(int literal) { return literal < 0; }
inline Variable& literalGetVar(State& s, int literal) {
	return s.vars[literalGetIdx(literal)];
}

inline bool literalIsFalse(State& s, int literal) {
	Variable& v = literalGetVar(s, literal);
	return (v.set && v.sign != literalGetSign(literal));
}

inline bool literalGetMark(State& s, int literal) {
	Variable& v = literalGetVar(s, literal);
	return v.mark;
}

inline void literalAddWatch(State& s, int literal, vector<int>& clause) {
	Variable& v = literalGetVar(s, literal);
	v.watches[literalGetSign(literal)].push_back(vector<int>(clause));
}

inline void literalSet(State& s, int literal, vector<int>* reason) {
	debugOut << "set " << literal << " with reason = ";
	if (reason == NULL) debugOut << "NULL" << endl;
	else  for (int x = 0; x < reason->size(); x++) debugOut << (*reason)[x] << " ";
	debugOut << endl;



	Variable& v = literalGetVar(s, literal);
	v.sign = literalGetSign(literal);
	v.set = true;
	v.dlevel = s.dlevel;
	v.reason = reason;
	s.trail.push_back(literal);
}

void satAddClause(State& s, vector<int>& clause) {
	debugOut << "Add clause of size " << clause.size() << endl;
	bool sign;
	if (clause.size() == 0) {
		s.empty = true;
		return;
	} else if (clause.size() == 1) {
		Variable& v = literalGetVar(s, clause[0]);
		sign = literalGetSign(clause[0]);
		if (v.unit) {
			if (sign != v.unit_sign)
				s.empty = true;
			return;
		}
		v.setUnit(sign);
		return;
	} else {
		literalAddWatch(s, clause[0], clause);
		literalAddWatch(s, clause[1], clause);
	}
}

int satSelectLiteral(State& s) {
	int M = 1;
	int N = s.vars.size() - 1;
	int i = M + (1 + N - M) * ((float)rand() / RAND_MAX);
	int i0 = i;
	while (s.vars[i].set) {
		i++;
		if (i >= s.vars.size())
			i = 1;
		if (i == i0) return 0;
	}
	int literal = ((float)rand() / RAND_MAX > 0.5 ? i : -i);
	return literal;
}

vector<int>* satBacktrack(State& s, vector<int>* reason) {
    vector<int> conflicts;

    // Level 0 failure; no work to do.
    if (s.dlevel == 0)
        return NULL;

    // Mark literals in reason:
    int count = 0;
    for (int i = 0; i < reason->size(); i++) {
        Variable& v = literalGetVar(s, (*reason)[i]);
        if (v.dlevel == 0)
            continue;
        v.mark = true;
        if (v.dlevel < s.dlevel)
            conflicts.push_back((*reason)[i]);
        else count++;
    }

    // Find the UIP and collect conflicts:
    int tlevel = s.trail.size() - 1;
    int literal;
    do {
        if (tlevel < 0)
            return NULL;
        literal = s.trail[tlevel--];
        Variable& v = literalGetVar(s, literal);
        v.set = false;
        if (!v.mark) continue;
        v.mark = false;
        count--;
        if (count <= 0) break;
        for (int i = 1; i < v.reason->size(); i++) {
            literal = (*v.reason)[i];
            Variable& w = literalGetVar(s, literal);
            if (w.mark || w.dlevel == 0)
                continue;
            if (w.dlevel < s.dlevel)
                conflicts.push_back(literal);
            else
                count++;
            w.mark = true;
        }
    }
    while (true);

    // Simplify the conflicts; create the no-good.
    vector<int>* nogood = new vector<int>;
    nogood->push_back(-literal);
    int blevel = 0;

    for (int i = 0; i < conflicts.size(); i++) {

        literal = conflicts[i];
        Variable& v = literalGetVar(s, literal);
        debugOut << "get literal: " << literal << endl;
        if (v.reason != NULL) {
            int k;
            for (k = 1; k < v.reason->size() && literalGetMark(s, (*v.reason)[k]); k++) {}
            if (k >= v.reason->size())
                continue;
        }

        nogood->push_back(literal);
        if (blevel < v.dlevel) {
            blevel = v.dlevel;
            (*nogood)[nogood->size() - 1] = (*nogood)[1];
            (*nogood)[1] = literal;
        }
    }

    // Unwind the trail:
    while (tlevel >= 0) {
        literal = s.trail[tlevel];
        Variable& v = literalGetVar(s, literal);
        if (v.dlevel <= blevel)
            break;
        v.set = false;
        tlevel--;
    }
    s.trail.resize(tlevel + 1);

    // Clear the marks:
    for (int i = 0; i < conflicts.size(); i++) {
        Variable& v = literalGetVar(s, conflicts[i]);
        v.mark = false;
    }

    // Add the no-good clause:
    satAddClause(s, *nogood);
    s.dlevel = blevel;
    if (s.empty)
        return NULL;

    return nogood;
}

bool satUnitPropagate(State&s, int literal, vector<int>* reason) {
	int curr, next;
	int restart;
	do {
		curr = s.trail.size();
		next = curr + 1;

		literalSet(s, literal, reason);

		restart = false;
		while (curr < next) {
			debugOut << "ploop with lit: " << literal << endl;

			literal = s.trail[curr];
			curr++;
			literal = -literal;
			Variable& v = literalGetVar(s, literal);
			vector<vector<int>> watch = v.watches[literalGetSign(literal)];

			for (int i = 0; i < watch.size(); i++) {

				vector<int>& clause = watch[i];
				int watch_idx = clause[0] == literal;
				int watch_lit = clause[watch_idx];
				int watch_sign = literalGetSign(watch_lit);
				Variable& w = literalGetVar(s, watch_lit);
				if (w.set && w.sign == watch_sign) continue;

				int j;
				for (j = 2; j < clause.size() && literalIsFalse(s, clause[j]); j++) {}
				if (j >= clause.size()) {

					if (!w.set) {
						if (watch_idx != 0) {
							clause[0] = watch_lit;
							clause[1] = literal;
						}
						literalSet(s, watch_lit, &clause);
						next++;
						continue;
					}
					reason = satBacktrack(s, &clause);

					if (reason == NULL || reason->size() == 0) return false;
					literal = (*reason)[0];
					restart = true;
					break;
				}
				debugOut << "pforfor end" << endl;
				int new_lit = clause[j];
				clause[int(!watch_idx)] = new_lit;
				clause[j] = literal;
				literalAddWatch(s, new_lit, clause);
				if (i == watch.size() - 1)
					watch.erase(watch.end() - 1);
				else {
					watch[i] = vector<int>(*(watch.end() - 1));
					watch.erase(watch.end() - 1);
					i--;
				}
			}
			if (restart) break;
		}
	} while (restart);
	return true;
}

bool satSolve(int num_vars, vector<vector<int>> clauses) {
	State s;
	for (int i = 0; i < num_vars; i++) s.vars.push_back(Variable());
	for (int i = 0; i < clauses.size(); i++)
		satAddClause(s, clauses[i]);
	if (s.empty) return false;

	for (int i = 1; i < s.vars.size(); i++) {
		Variable v = s.vars[i];
		if (v.unit) {
			int literal = (v.unit_sign ? -i : i);
			if (!satUnitPropagate(s, literal, NULL))
				return false;
		}
	}

	debugOut << "Befor main loop" << endl;
	for (s.dlevel = 1; true; s.dlevel++) {
		int literal = satSelectLiteral(s);
		if (literal == 0)
			return true;

		if (!satUnitPropagate(s, literal, NULL))
			return false;
	}
	return true;
}

void loadCNF(FILE* stream, int& num_vars, vector<vector<int>>& clauses) {
	int num_clauses;
	size_t input_len = 100;
	char* input_buf;
	input_buf = (char *)malloc(input_len * sizeof(char));

	while (getline(&input_buf, &input_len, stream) != -1) {
		if (input_buf[0] == 'c') continue;
		else if (input_buf[0] == 'p') {
			sscanf(input_buf, "p cnf %d %d", &num_vars, &num_clauses);
			if (num_vars <= 0 || num_clauses <= 0) {
				fprintf(stderr, "Invalid value for propositions or clauses.\n");
			}
			break;
		}
	}

	int *cnf_buffer = new int[num_vars * num_clauses];

	vector<int>clause;

	debugOut << "Number of propositions: " << num_vars << endl;
	debugOut << "Number of clauses: " << num_clauses << endl;

	int valprop;
	while (fscanf(stream, "%d", &valprop) > 0) {
		if (valprop == 0) {
			clauses.push_back(clause);
			clause.clear();
		} else {
			clause.push_back(valprop);
		}
	}

	for (int i = 0; i < clauses.size(); i++) {
		auto v = clauses[i];
		debugOut << "Clause " << i + 1 << ":";
		for (int j = 0; j < v.size(); j++) {
			debugOut << " " << v[j];
		}
		debugOut << endl;
	}

}

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

	int num_vars;
	vector<vector<int>> clauses;

	loadCNF(stream, num_vars, clauses);
	if (satSolve(num_vars, clauses)) {
		cout << "SAT" << endl;
	} else {
		cout << "UNSAT" << endl;
	}

	return 0;
}


