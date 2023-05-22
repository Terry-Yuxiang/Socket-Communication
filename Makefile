CXXFLAGS = -ggdb -Wall -std=c++11

all: client.cpp serverM.cpp serverA.cpp serverB.cpp
	g++ $(CXXFLAGS) -o serverM serverM.cpp
	g++ $(CXXFLAGS) -o serverA serverA.cpp
	g++ $(CXXFLAGS) -o serverB serverB.cpp
	g++ $(CXXFLAGS) -o client client.cpp



serverM: serverM.cpp serverM.h
	g++ $(CXXFLAGS) -o serverM serverM.cpp

client: client.cpp client.h
	g++ $(CXXFLAGS) -o client client.cpp

serverA: serverA.cpp serverA.h
	g++ $(CXXFLAGS) -o serverA serverA.cpp

serverB: serverB.cpp serverB.h
	g++ $(CXXFLAGS) -o serverB serverB.cpp

clean:
	-rm -f serverM client serverA serverB