ADDh = ../headers/
USEc= -std=c++11 -I $(ADDh) -Wall -Wfatal-errors	
Boost= -lboost_unit_test_framework
vpath %.h $(ADDh)

all: correctnessT1 correctnessT2 timingT1 timingT2

personal: primeFac primeT


primeFac:  correctnessTests.cpp primeFactorisation.o
	g++ $(USEc) $^ -o primeFac $(Boost)
	
primeT:  timingTests.cpp primeFactorisation.o
	g++ $(USEc) $^ -o primeT

correctnessT1: correctnessTests.cpp primeFactorisation-BestStudent.o
	g++ $(USEc) $^ -o correctnessT1 $(Boost)

correctnessT2: correctnessTests.cpp primeFactorisation-Reference.o
	g++ $(USEc) $^ -o correctnessT2 $(Boost)

timingT1 : timingTests.cpp primeFactorisation-BestStudent.o
	g++ $(USEc) $^ -o timingT1

timingT2: timingTests.cpp primeFactorisation-Reference.o
	g++ $(USEc) $^ -o timingT2
	
primeFactorisation.o: primeFactorisation.cpp primeFactorisation.h
	g++ $(USEc) -c primeFactorisation.cpp -o primeFactorisation.o


clear:
	rm -f correctnessT1 correctnessT2 timingT1 timingT2 primeFac primeT
