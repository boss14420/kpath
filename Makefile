all: kpath kpath-parallel kpath-noloadbalancing

CC ?= gcc
CXX ?= g++

CXXFLAGS_BASE = -Wall -std=c++11 -pedantic-errors -pipe
CXXFLAGS_DEBUG = $(CXXFLAGS_BASE) -g #-pg
CXXFLAGS = $(CXXFLAGS_BASE) -DNDEBUG -Ofast -march=native -fomit-frame-pointer

kpath: kpath.o
	$(CXX) $(CXXFLAGS) -o $@ $<

kpath-debug: kpath-debug.o
	$(CXX) $(CXXFLAGS_DEBUG) -o $@ $<

kpath-parallel: kpath-parallel.o
	$(CXX) $(CXXFLAGS) -fopenmp -o $@ $<

kpath-parallel-debug: kpath-parallel-debug.o
	$(CXX) $(CXXFLAGS_DEBUG) -fopenmp -o $@ $<

kpath.o: kpath.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

kpath-debug.o: kpath.cc
	$(CXX) $(CXXFLAGS_DEBUG) -c -o $@ $<

kpath-parallel.o: kpath.cc
	$(CXX) $(CXXFLAGS) -fopenmp -c -o $@ $<

kpath-parallel-debug.o: kpath.cc
	$(CXX) $(CXXFLAGS_DEBUG) -fopenmp -c -o $@ $<

kpath-noloadbalancing: kpath-noloadbalancing.o
	$(CXX) $(CXXFLAGS) -DNO_LOAD_BALANCING -fopenmp -o $@ $<

kpath-noloadbalancing.o: kpath.cc
	$(CXX) $(CXXFLAGS) -DNO_LOAD_BALANCING -fopenmp -c -o $@ $<

kpath-noloadbalancing-debug: kpath-noloadbalancing-debug.o
	$(CXX) $(CXXFLAGS_DEBUG) -DNO_LOAD_BALANCING -fopenmp -o $@ $<

kpath-noloadbalancing-debug.o: kpath.cc
	$(CXX) $(CXXFLAGS_DEBUG) -DNO_LOAD_BALANCING -fopenmp -c -o $@ $<

.PHONY: clean

clean:
	rm -rf *.o

release: kpath kpath-parallel kpath-noloadbalancing

debug: kpath-debug kpath-parallel-debug kpath-noloadbalancing-debug

serial: kpath kpath-debug kpath-noloadbalancing-debug

parallel: kpath-parallel kpath-parallel-debug

noloadbalancing: kpath-noloadbalancing kpath-noloadbalancing-debug
