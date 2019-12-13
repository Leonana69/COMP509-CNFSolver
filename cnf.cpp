/* 
 * Author: Guojun Chen, gc34@rice.edu
 * CNF solver
 */

#include "cnf.h"
#include <algorithm>

#ifndef DEBUG
extern NullBuffer nullbuffer;
extern std::ostream null_stream;
#endif

void CNF::satAddClause(vector<int>& clause) {
	bool sign;
	if (clause.size() == 0) {
		s.empty = true;
		return;
	} else if (clause.size() == 1) {
		Proposition& v = propGetVar(clause[0]);
		sign = propGetSign(clause[0]);
		// if two unary clauses have the confilict proposition
		if (v.unit) {
			if (sign != v.unit_sign)
				s.empty = true;
			return;
		}
		v.setUnit(sign);
		return;
	} else if (clause.size() == 3) {
		s.twoClauseOccurance[propGetIdx(clause[0])].cnt++;
		s.twoClauseOccurance[propGetIdx(clause[1])].cnt++;
		// s.twoClauseOccurance[propGetIdx(clause[2])].cnt++;
	}


	for (int i = 0; i < clause.size(); i++) {
		s.allClauseOccurance[propGetIdx(clause[i])].cnt++;
		if (clause[0] > 0)
			s.ocrCnt[propGetIdx(clause[i])]++;
	}
	
	propAddClause(clause[0], clause);
	propAddClause(clause[1], clause);
}

int CNF::satSelectorTwoClause() {
	calls++;
	
	for (int i = 1; i <= s.num_props; i++) {
		if (!s.props[s.twoClauseOccurance[i].prop].set) {
			int j = s.twoClauseOccurance[i].prop;
			return ((float)rand() / RAND_MAX > 0.5 ? j : -j);
		}
	}
	return 0;
}

int CNF::satSelectorAppcnt() {
	calls++;
	
	for (int i = 1; i <= s.num_props; i++) {
		if (!s.props[s.allClauseOccurance[i].prop].set) {
			int j = s.allClauseOccurance[i].prop;
			if (s.ocrCnt[j] * 2 > s.allClauseOccurance[i].cnt)
				return ((float)rand() / RAND_MAX > 0.2 ? j : -j);
			else return ((float)rand() / RAND_MAX > 0.8 ? j : -j);
		}
	}
	return 0;
}


// random selector
int CNF::satSelectorRandom() {
	calls++;

	int i = rand() % s.num_props + 1;
	int i0 = i;
	while (s.props[i].set) {
		i = (i % s.num_props) + 1;
		if (i == i0) return 0; // all proposition has been set
	}
	int prop = ((float)rand() / RAND_MAX > 0.5 ? i : -i); // set random value
	return prop;
}

vector<int> CNF::satBacktrack(vector<int>& reason) {
    vector<int> conflicts;

    // Level 0 failure; no work to do.
    if (s.dlevel == 0)
        return vector<int>();

    // Mark props in reason:
    int count = 0;
    for (int i = 0; i < reason.size(); i++) {
        Proposition& v = propGetVar(reason[i]);
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
        Proposition& v = propGetVar(prop);
        v.set = false;
        if (!v.mark) continue;
        v.mark = false;
        count--;
        if (count <= 0) break;
        for (int i = 1; i < v.reason.size(); i++) {
            prop = v.reason[i];
            Proposition& w = propGetVar(prop);
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
        Proposition& v = propGetVar(prop);
        if (v.reason != vector<int>()) {
            int k;
            for (k = 1; k < v.reason.size() && propGetMark(v.reason[k]); k++) {}
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
        Proposition& v = propGetVar(prop);
        if (v.dlevel <= blevel)
            break;
        v.set = false;
        tlevel--;
    }
    s.trail.resize(tlevel + 1);

    // Clear the marks:
    for (int i = 0; i < conflicts.size(); i++) {
        Proposition& v = propGetVar(conflicts[i]);
        v.mark = false;
    }

    // Add the no-good clause:
    satAddClause(nogood);
    s.dlevel = blevel;
    if (s.empty)
        return vector<int>();

    return nogood;
}

bool CNF::satUnitPropagate(int prop) {
	vector<int> reason;
	int curr, next;
	int restart;
	do {
		curr = s.trail.size();
		next = curr + 1;

		propSet(prop, reason);

		restart = false;
		while (curr < next) {
			prop = s.trail[curr];
			curr++;
			prop = -prop;
			Proposition& v = propGetVar(prop);
			vector<vector<int>>& watch = v.clause[propGetSign(prop)];

			for (int i = 0; i < watch.size(); i++) {

				vector<int>& clause = watch[i];
				int watch_idx = clause[0] == prop;
				int watch_lit = clause[watch_idx];
				int watch_sign = propGetSign(watch_lit);
				Proposition& w = propGetVar(watch_lit);
				if (w.set && w.sign == watch_sign) continue;

				int j;
				for (j = 2; j < clause.size() && propIsFalse(clause[j]); j++) {}
				if (j >= clause.size()) {

					if (!w.set) {
						if (watch_idx != 0) {
							clause[0] = watch_lit;
							clause[1] = prop;
						}
						propSet(watch_lit, clause);
						next++;
						continue;
					}
					reason = satBacktrack(clause);

					if (reason.size() == 0) return false;
					prop = reason[0];
					restart = true;
					break;
				}

				int new_lit = clause[j];
				clause[int(!watch_idx)] = new_lit;
				clause[j] = prop;
				propAddClause(new_lit, clause);
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

const CNF::fptr CNF::satSelector[3] = {
    &CNF::satSelectorRandom,
    &CNF::satSelectorTwoClause,
    &CNF::satSelectorAppcnt
};

CNF::CNF(int num_props, vector<vector<int>>& clauses, int selector) {
	calls = 0;
	s.init();
	s.num_props = num_props;
	// srand(time(0));
	for (int i = 0; i <= num_props; i++) s.props.push_back(Proposition());
	for (int i = 0; i <= num_props; i++) s.twoClauseOccurance.push_back(Occurance(i));
	for (int i = 0; i <= num_props; i++) s.allClauseOccurance.push_back(Occurance(i));
	for (int i = 0; i <= num_props; i++) s.ocrCnt.push_back(0);

	for (int i = 0; i < clauses.size(); i++)
		satAddClause(clauses[i]);
	if (s.empty) {
		result = false;
		return;
	}

	sort(s.twoClauseOccurance.begin() + 1, s.twoClauseOccurance.end());
	sort(s.allClauseOccurance.begin() + 1, s.allClauseOccurance.end());
	// sort(s.ocrCnt.begin() + 1, s.ocrCnt.end());

	for (int i = 1; i <= s.num_props; i++) {
		Proposition& v = s.props[i];
		if (v.unit) {
			int prop = (v.unit_sign ? -i : i);
			if (!satUnitPropagate(prop)) {
				result = false;
				return;
			}
		}
	}

	s.dlevel = 0;
	while (++s.dlevel) {
		int prop = (this->*satSelector[selector])();
		if (prop == 0) {
			result = true;
			return;
		}

		if (!satUnitPropagate(prop)) {
			result = false;
			return;
		}
	}
}
