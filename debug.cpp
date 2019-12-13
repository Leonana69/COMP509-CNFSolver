#include "debug.h"
	
#ifndef DEBUG
NullBuffer nullbuffer;
std::ostream null_stream(&nullbuffer);
#endif