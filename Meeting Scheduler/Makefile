# build an executable named serverA/B/M/client from .cpp files

.PHONY: all clean

all: serverM serverA serverB client

serverM: serverM.cpp 
	g++ -std=c++11 -o serverM serverM.cpp 
serverA: serverA.cpp  
	g++ -std=c++11 -o serverA serverA.cpp  
serverB: serverB.cpp  
	g++ -std=c++11 -o serverB serverB.cpp  
client: client.cpp  
	g++ -std=c++11 -o client client.cpp
    
clean:  
	rm serverM
	rm serverA
	rm serverB
	rm client

