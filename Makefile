CXX = g++
CXXFLAGS = -Wall -std=c++17

all: main

main: source.o
	$(CXX) $(CXXFLAGS) -o main source.o

source.o: source.cpp source.h
	$(CXX) $(CXXFLAGS) -c source.cpp

clean:
	rm -f *.o main
