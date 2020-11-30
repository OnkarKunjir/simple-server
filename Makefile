HEADER_DIR = include/
SRC_DIR = src/
INCS = -I$(HEADER_DIR)

build: main.o server.o
	g++ main.o server.o -o server.out

main.o: main.cpp $(HEADER_DIR)server.hpp
	g++ -c main.cpp $(INCS)

server.o: $(HEADER_DIR)server.hpp
	g++ -c $(SRC_DIR)server.cpp $(INCS)

clean:
	rm *.o
	rm server.out
