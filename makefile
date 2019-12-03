CC = g++
CFLAGS = -I

CNF: main.o
	$(CC) -o CNF main.o -std=c++11

CNF2: main2.o
	$(CC) -o CNF2 main2.o -std=c++11

main.o: main.cpp
	$(CC) -c main.cpp -std=c++11

main2.o: main2.cpp
	$(CC) -c main2.cpp -std=c++11

clean:
	-rm $(objects) *.o CNF CNF2

