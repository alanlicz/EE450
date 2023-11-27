CXX = g++

all: servermain serverA serverB serverC

servermain: servermain.cc
	$(CXX) servermain.cc -o servermain

serverA: serverA.cc
	$(CXX) serverA.cc -o serverA

serverB: serverB.cc
	$(CXX) serverB.cc -o serverB

serverC: serverC.cc
	$(CXX) serverC.cc -o serverC

clean:
	rm -f servermain serverA serverB serverC