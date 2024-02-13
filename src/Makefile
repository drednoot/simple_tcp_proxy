CXX=gcc -lstdc++
FLAGS=-g

all: proxy

socket.o: socket.cc
	$(CXX) $(FLAGS) -c socket.cc

server.o: server.cc
	$(CXX) $(FLAGS) -c server.cc

logger.o: logger.cc
	$(CXX) $(FLAGS) -c logger.cc

proxy: socket.o server.o logger.o main.cc
	$(CXX) $(FLAGS) socket.o server.o logger.o main.cc -o proxy

clean:
	rm -rf *.o
	rm -rf proxy
