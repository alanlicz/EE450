CXX = g++

all: servermain servermain_tcp serverA serverB serverC student1 

servermain: servermain.cc
	$(CXX) servermain.cc -o servermain

servermain_tcp: servermain_tcp.cc
	$(CXX) servermain_tcp.cc -o servermain_tcp

serverA: serverA.cc
	$(CXX) serverA.cc -o serverA

serverB: serverB.cc
	$(CXX) serverB.cc -o serverB

serverC: serverC.cc
	$(CXX) serverC.cc -o serverC

student1: student1.cc
	$(CXX) student1.cc -o student1

clean:
	rm -f servermain servermain_tcp serverA serverB serverC student1 