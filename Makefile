HEADER_DIR = include/
SRC_DIR = src/
INCS = -I$(HEADER_DIR)

build: simple_server

run: build
	./simple_server

simple_server: main.o
	g++ main.o server.o -o simple_server

main.o: main.cpp server.o
	g++ -c main.cpp $(INCS)

server.o: $(SRC_DIR)server.cpp $(HEADER_DIR)server.hpp
	g++ -c $(SRC_DIR)server.cpp $(INCS)

clean:
	rm *.o
