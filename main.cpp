#include "include/server.hpp"
#include "include/utils.hpp"

#include <string>
#include <vector>
#include<cstring>
#include <iostream>
const int PORT = 8080;

int main(){
    SimpleServer server("127.0.0.1", 8080, 5);
    server.init();
    server.serve();
    return 0;
}
