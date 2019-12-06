CC = g++
CFLAGS = -I

CNF: main.o
	$(CC) -o CNF main.o -std=c++11

main.o: main.cpp
	$(CC) -c main.cpp -std=c++11

clean:
	-rm $(objects) *.o CNF

