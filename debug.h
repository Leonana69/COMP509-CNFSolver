/* 
 * Author: Guojun Chen, gc34@rice.edu
 * Debug module
 */
#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>

#define DEBUGG

#ifdef DEBUG
	#define debugOut cout
#else
	class NullBuffer: public std::streambuf {
	public:
	  int overflow(int c) { return c; }
	};
	#define debugOut null_stream
#endif

#endif