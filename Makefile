CXX=g++
CXXFLAGS=-std=c++17 -O3 -march=native -g
LDFLAGS=-g
LDLIBS=-lnetcdf_c++4 -lnetcdf

all: analyzewave
	
# Core modules compliation {{{
analyzewave.o: analyzewave.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $< 

analyzewave: analyzewave.o 
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)
#}}}

# Module tests compilation {{{

./int_test: int_test.o
	$(CXX) $(LDFLAGS) -o $@ $^   -lCatch2Main -lCatch2 #-lboost_unit_test_framework

int_test.o: int_test.cpp
	$(CXX) -c $(CXXFLAGS) -o int_test.o int_test.cpp

#}}}

clean:
	$(RM) analyzewave.o mpianalyzewave.o analyzewave mpianalyzewave int_test.o int_test
