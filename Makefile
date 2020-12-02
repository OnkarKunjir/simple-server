HEADER_DIR = include/
SRC_DIR = src/
INCS = -I$(HEADER_DIR)

build: simple_server

run: build
	./simple_server

simple_server: main.o
	g++ main.o utils.o server.o -o simple_server

main.o: main.cpp server.o utils.o
	g++ -c main.cpp $(INCS)

server.o: $(SRC_DIR)server.cpp $(HEADER_DIR)server.hpp
	g++ -c $(SRC_DIR)server.cpp $(INCS)

utils.o: $(SRC_DIR)utils.cpp $(HEADER_DIR)utils.hpp
	g++ -c $(SRC_DIR)utils.cpp

clean:
	rm *.o
