CXX = g++

all: servermain client serverTest serverA serverB

servermain: servermain.cc
	$(CXX) servermain.cc -o servermain

client: client.cc
	$(CXX) client.cc -o client

serverTest: serverTest.cc
	$(CXX) serverTest.cc -o serverTest

serverA: serverA.cc
	$(CXX) serverA.cc -o serverA

serverB: serverB.cc
	$(CXX) serverB.cc -o serverB

clean:
	rm -f servermain client serverTest clientTest	