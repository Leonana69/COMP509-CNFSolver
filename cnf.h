/* 
 * Author: Guojun Chen, gc34@rice.edu
 * CNF solver
 */

#ifndef CNF_H
#define CNF_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <ctime>
#include <vector>
#include "debug.h"

using namespace std;

struct Proposition {
	bool set;			// whether it has been assigned a value
	bool sign;			// the value
	bool mark;			// mark
	bool unit;			// clause of size 1
	bool unit_sign;		// sign for the above 1-clause
	int dlevel;
	vector<int> reason;
	vector<vector<int>> clause[2];
	void setUnit(bool _sign) {
		unit = true;
		unit_sign = _sign;
	}
	Proposition() {
		set = false;
		sign = false;
		mark = false;
		unit = false;
		unit_sign = false;
		dlevel = 0;
		reason.clear();
		clause[0].clear();
		clause[1].clear();
	}
};

struct Occurance {
	int prop, cnt;
	Occurance(int _prop) { prop = _prop, cnt = 0; }
	bool operator<(const Occurance& o) {
		return cnt > o.cnt;
	}
};

struct State {
	bool empty;
	vector<Proposition> props;
	int num_props;
	vector<Occurance> twoClauseOccurance;
	vector<Occurance> allClauseOccurance;
	vector<int> ocrCnt;
	vector<int> trail;
	int dlevel;
	int tlevel;
	State() {
		num_props = 0;
		empty = false;
		dlevel = tlevel = 0;
		props.clear();
		trail.clear();
		twoClauseOccurance.clear();
		allClauseOccurance.clear();
		ocrCnt.clear();
	}
	void init() {
		num_props = 0;
		empty = false;
		dlevel = tlevel = 0;
		props.clear();
		trail.clear();
		twoClauseOccurance.clear();
		allClauseOccurance.clear();
		ocrCnt.clear();
	}
};

class CNF {
private:
	State s;
	bool result;
	int selector;
	int calls;
	typedef int (CNF::*fptr)();
	const static fptr satSelector[3];
	
public:
	int numberOfCalls() { return calls; }
	bool satisfiable() { return result; }
	int numOfVars() { return s.num_props; }
	int propGetIdx(int prop) { return prop < 0 ? -prop : prop; }
	int propGetSign(int prop) { return prop < 0; }
	Proposition& propGetVar(int prop) {
		if (prop == 0) cerr << "error" << endl;
		return s.props[propGetIdx(prop)];
	}

	bool propIsFalse(int prop) {
		Proposition& v = propGetVar(prop);
		return (v.set && v.sign != propGetSign(prop));
	}

	bool propGetMark(int prop) {
		Proposition& v = propGetVar(prop);
		return v.mark;
	}

	void propAddClause(int prop, vector<int>& clause) {
		Proposition& v = propGetVar(prop);
		v.clause[propGetSign(prop)].push_back(vector<int>(clause));
	}

	void propSet(int prop, vector<int>& reason) {
		Proposition& v = propGetVar(prop);
		v.sign = propGetSign(prop);
		v.set = true;
		v.dlevel = s.dlevel;
		v.reason.clear();
		v.reason = reason;
		s.trail.push_back(prop);
	}

	void satAddClause(vector<int>& clause);
	int satSelectorRandom();
	int satSelectorTwoClause();
	int satSelectorAppcnt();
	vector<int> satBacktrack(vector<int>& reason);
	bool satUnitPropagate(int prop);
	CNF(int num_props, vector<vector<int>>& clauses, int selector);
};

#endif
