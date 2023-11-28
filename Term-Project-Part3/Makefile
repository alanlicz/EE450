CXX = g++

all: serverA serverB serverC servermain

serverA: serverA.cc
	$(CXX) -o serverA serverA.cc

serverB: serverB.cc
	$(CXX) -o serverB serverB.cc

serverC: serverC.cc
	$(CXX) -o serverC serverC.cc

servermain: servermain.cc
	$(CXX) -o servermain servermain.cc

clean:
	rm -f serverA serverB serverC servermain