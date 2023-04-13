CXX=g++
CXXFLAGS=-std=c++17 -O0 -march=native -g
LDFLAGS=-g
LDLIBS=-lnetcdf_c++4 -lnetcdf -lmpi_cxx -lmpi

all: analyzewave
	
# Core modules compliation {{{
analyzewave.o: analyzewave.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $< 

analyzewave: analyzewave.o 
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)
#}}}

# Module tests compilation {{{

int_test.o: int_test.cpp
	$(CXX) -c $(CXXFLAGS) -o int_test.o int_test.cpp

int_test: int_test.o
	$(CXX) $(LDFLAGS) -o $@ $^   -lCatch2Main -lCatch2 #-lboost_unit_test_framework
	make analyzewave

#}}}

run:
	./analyzewave precisewave.nc energies.tsv 999999

clean:
	$(RM) analyzewave.o mpianalyzewave.o analyzewave mpianalyzewave int_test.o int_test
