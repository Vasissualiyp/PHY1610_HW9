CXX=g++
CXXFLAGS=-std=c++17 -O3 -march=native -g
LDFLAGS=-g
LDLIBS=-lnetcdf_c++4 -lnetcdf

all: analyzewave

analyzewave.o: analyzewave.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $< 

analyzewave: analyzewave.o 
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	$(RM) analyzewave.o mpianalyzewave.o analyzewave mpianalyzewave
