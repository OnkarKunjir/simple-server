#include "include/server.hpp"

const int PORT = 8080;


int main(){
    SimpleServer server("127.0.0.1", 8080, 5);
    server.init();
    server.serve();

    return 0;
}
