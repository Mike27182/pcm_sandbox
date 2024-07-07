all: pmc_sandbox

pmc_sandbox: main.cpp pmc.h pmc.cpp Makefile
	g++ main.cpp pmc.cpp -O3 -g -march=native -mavx2 -std=c++23 -lhugetlbfs -o pmc_sandbox -Wall -Wextra -pedantic -Wno-unused-parameter -fmax-errors=5 -DNDEBUG

clean:
	rm pmc_sandbox

