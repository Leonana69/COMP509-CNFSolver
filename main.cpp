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

#define DEBUGG

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
	vector<int> reason;
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
		reason.clear();
		watches[0].clear();
		watches[1].clear();
	}
};

struct State {
	bool empty;
	vector<Variable> vars;
	int num_vars;
	vector<int> clauses;
	vector<int> trail;
	int dlevel;
	int tlevel;
	State() {
		num_vars = 0;
		empty = false;
		dlevel = tlevel = 0;
		vars.clear();
		clauses.clear();
		trail.clear();
	}
};

inline int propGetIdx(int prop) { return prop < 0 ? -prop : prop; }
inline int propGetSign(int prop) { return prop < 0; }
inline Variable& propGetVar(State& s, int prop) {
	if (prop == 0) cerr << "error" << endl;
	return s.vars[propGetIdx(prop)];
}

inline bool propIsFalse(State& s, int prop) {
	Variable& v = propGetVar(s, prop);
	return (v.set && v.sign != propGetSign(prop));
}

inline bool propGetMark(State& s, int prop) {
	Variable& v = propGetVar(s, prop);
	return v.mark;
}

inline void propAddWatch(State& s, int prop, vector<int>& clause) {
	Variable& v = propGetVar(s, prop);
	v.watches[propGetSign(prop)].push_back(vector<int>(clause));
}

inline void propSet(State& s, int prop, vector<int>& reason) {
	Variable& v = propGetVar(s, prop);
	v.sign = propGetSign(prop);
	v.set = true;
	v.dlevel = s.dlevel;
	v.reason.clear();
	v.reason = reason;
	s.trail.push_back(prop);
}

void satAddClause(State& s, vector<int>& clause) {
	bool sign;
	if (clause.size() == 0) {
		s.empty = true;
		return;
	} else if (clause.size() == 1) {
		Variable& v = propGetVar(s, clause[0]);
		sign = propGetSign(clause[0]);
		// if two unary clauses have the confilict proposition
		if (v.unit) {
			if (sign != v.unit_sign)
				s.empty = true;
			return;
		}
		v.setUnit(sign);
		return;
	} else {
		// for (int i = 0; i < clause.size(); i++)
		// 	propAddWatch(s, clause[i], clause);
		propAddWatch(s, clause[0], clause);
		propAddWatch(s, clause[1], clause);
	}
}

int satSelectLiteral(State& s) {
	int N = s.num_vars; // change
	int i = 1 + (N - 1) * ((float)rand() / RAND_MAX);
	int i0 = i;
	while (s.vars[i].set) {
		i++;
		if (i > N) i = 1;
		if (i == i0) return 0;
	}
	int prop = ((float)rand() / RAND_MAX > 0.5 ? i : -i);
	return prop;
}

vector<int> satBacktrack(State& s, vector<int>& reason) {
    vector<int> conflicts;

    // Level 0 failure; no work to do.
    if (s.dlevel == 0)
        return vector<int>();

    // Mark props in reason:
    int count = 0;
    for (int i = 0; i < reason.size(); i++) {
        Variable& v = propGetVar(s, reason[i]);
        if (v.dlevel == 0)
            continue;
        v.mark = true;
        if (v.dlevel < s.dlevel)
            conflicts.push_back(reason[i]);
        else count++;
    }

    // Find the UIP and collect conflicts:
    int tlevel = s.trail.size() - 1;
    int prop;
    do {
        if (tlevel < 0)
            return vector<int>();
        prop = s.trail[tlevel--];
        Variable& v = propGetVar(s, prop);
        v.set = false;
        if (!v.mark) continue;
        v.mark = false;
        count--;
        if (count <= 0) break;
        for (int i = 1; i < v.reason.size(); i++) {
            prop = v.reason[i];
            Variable& w = propGetVar(s, prop);
            if (w.mark || w.dlevel == 0)
                continue;
            if (w.dlevel < s.dlevel)
                conflicts.push_back(prop);
            else
                count++;
            w.mark = true;
        }
    }
    while (true);

    // Simplify the conflicts; create the no-good.
    vector<int> nogood;
    nogood.push_back(-prop);
    int blevel = 0;

    for (int i = 0; i < conflicts.size(); i++) {

        prop = conflicts[i];
        Variable& v = propGetVar(s, prop);
        if (v.reason != vector<int>()) {
            int k;
            for (k = 1; k < v.reason.size() && propGetMark(s, v.reason[k]); k++) {}
            if (k >= v.reason.size())
                continue;
        }

        nogood.push_back(prop);
        if (blevel < v.dlevel) {
            blevel = v.dlevel;
            nogood[nogood.size() - 1] = nogood[1];
            nogood[1] = prop;
        }
    }

    // Unwind the trail:
    while (tlevel >= 0) {
        prop = s.trail[tlevel];
        Variable& v = propGetVar(s, prop);
        if (v.dlevel <= blevel)
            break;
        v.set = false;
        tlevel--;
    }
    s.trail.resize(tlevel + 1);

    // Clear the marks:
    for (int i = 0; i < conflicts.size(); i++) {
        Variable& v = propGetVar(s, conflicts[i]);
        v.mark = false;
    }

    // Add the no-good clause:
    satAddClause(s, nogood);
    s.dlevel = blevel;
    if (s.empty)
        return vector<int>();

    return nogood;
}

bool satUnitPropagate(State&s, int prop) {
	vector<int> reason;
	int curr, next;
	int restart;
	do {
		curr = s.trail.size();
		next = curr + 1;

		propSet(s, prop, reason);

		restart = false;
		while (curr < next) {
			prop = s.trail[curr];
			curr++;
			prop = -prop;
			Variable& v = propGetVar(s, prop);
			vector<vector<int>>& watch = v.watches[propGetSign(prop)];

			for (int i = 0; i < watch.size(); i++) {

				vector<int>& clause = watch[i];
				int watch_idx = clause[0] == prop;
				int watch_lit = clause[watch_idx];
				int watch_sign = propGetSign(watch_lit);
				Variable& w = propGetVar(s, watch_lit);
				if (w.set && w.sign == watch_sign) continue;

				int j;
				for (j = 2; j < clause.size() && propIsFalse(s, clause[j]); j++) {}
				if (j >= clause.size()) {

					if (!w.set) {
						if (watch_idx != 0) {
							clause[0] = watch_lit;
							clause[1] = prop;
						}
						propSet(s, watch_lit, clause);
						next++;
						continue;
					}
					reason = satBacktrack(s, clause);

					if (reason.size() == 0) return false;
					prop = reason[0];
					restart = true;
					break;
				}

				int new_lit = clause[j];
				clause[int(!watch_idx)] = new_lit;
				clause[j] = prop;
				propAddWatch(s, new_lit, clause);
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

State s;

bool satSolve(int num_vars, vector<vector<int>> clauses) {
	s.num_vars = num_vars;
	for (int i = 0; i <= num_vars; i++) s.vars.push_back(Variable());
	for (int i = 0; i < clauses.size(); i++)
		satAddClause(s, clauses[i]);
	if (s.empty) return false;

	for (int i = 1; i <= s.num_vars; i++) {
		Variable& v = s.vars[i];
		if (v.unit) {
			int prop = (v.unit_sign ? -i : i);
			if (!satUnitPropagate(s, prop))
				return false;
		}
	}

	s.dlevel = 0;
	while (++s.dlevel) {
		int prop = satSelectLiteral(s);
		if (prop == 0) {
			return true;
		}

		if (!satUnitPropagate(s, prop))
			return false;
	}
	return true;
}

void loadCNF(FILE* stream, int& num_vars, vector<vector<int>>& clauses) {
	int num_clauses;
	clauses.clear();
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

	

	int valprop;
	while (fscanf(stream, "%d", &valprop) > 0) {
		if (valprop == 0) {
			clauses.push_back(clause);
			clause.clear();
		} else {
			clause.push_back(valprop);
		}
	}

	cout << "Number of propositions: " << num_vars << endl;
	cout << "Number of clauses: " << clauses.size() << endl;

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
		int sum = 0;
		int set = 0;
		for (int i = 1; i <= num_vars; i++) {
			cout << "p" << i << ": " << !propGetVar(s, i).sign << endl;
			sum += propIsFalse(s, i);
			set += propGetVar(s, i).set;
		}
		cout << set << endl;
		cout << sum << endl;

	} else {
		cout << "UNSAT" << endl;
	}

	return 0;
}


