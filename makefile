CC = g++
CFLAGS = -I

CNF: cnf.o debug.o cnf.h debug.h main.cpp
	$(CC) -o CNF main.cpp cnf.o debug.o -std=c++11

GEN: generator.cpp
	$(CC) -o GEN generator.cpp -std=c++11

cnf.o: cnf.cpp cnf.h debug.h
	$(CC) -c cnf.cpp -std=c++11

debug.o: debug.h debug.cpp
	$(CC) -c debug.cpp -std=c++11

clean:
	-rm $(objects) *.o CNF GEN
