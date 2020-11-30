#include <iostream>
#include <cstring>
#include <string>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "server.hpp"

SimpleServer::SimpleServer(const char* ip_address, int port, int backlog) {
    address.sin_port = htons(port);
    address.sin_family = AF_INET;
    inet_aton(ip_address, &address.sin_addr);
}

int SimpleServer::init() {
    // function creates new socket sets resue address and port flags and then binds the socket with provided ip address and port number.
    // returns -1 if something went wrong else 0.
    if((file_descriptor = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        return -1;

    int flag = 1;  // flag to set the options to true.
    if(setsockopt(file_descriptor, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &flag, sizeof(flag)) == -1)
        return -1;

    if(bind(file_descriptor, (const struct sockaddr*)&address, sizeof(address)) == -1)
        return -1;

    return 0;
}

ConnectionInfo SimpleServer::accept_connection() {
    // function accepts connection form clients.
    struct ConnectionInfo client;
    int address_len = sizeof(client.address);
    client.file_descriptor = accept(file_descriptor, (struct sockaddr*)&client.address, (socklen_t *)&address_len);
    if(client.file_descriptor == -1)
        std::cout<<"Failed to accept connection\n";
    return client;
}

int SimpleServer::receive(const ConnectionInfo &connection, int buffer_size = 1000){
    // function to receive message from client.
    print_connection_info(connection);

    char buffer[buffer_size];
    int bytes_received = 1;

    char demo_msg[] = "hello ba ba boi";
    while(bytes_received > 0){
        bytes_received = recv(connection.file_descriptor, buffer, buffer_size, 0);
        if(bytes_received == -1){
            std::cout<<"[ERROR] failed to receive message\n";
            return -1;
        }
        else if(bytes_received > 0){
            buffer[bytes_received] = 0;
        }
        else{
            buffer[buffer_size] = 0;
        }
        std::cout<<"[Bytes] "<<bytes_received<<std::endl;
        std::cout<<"[Message] "<<buffer<<std::endl;

        send_message(connection, demo_msg, sizeof(demo_msg));
        break;
    }
    return 0;
}


void SimpleServer::send_message(const ConnectionInfo &connection, const char* message, int message_len){
    send(connection.file_descriptor, message, message_len, 0);
}

void SimpleServer::print_connection_info(const ConnectionInfo &connection) {
    // print the information related to connection
    // prints only ip address for now.
    std::cout<<"[Connection from] "<<inet_ntoa(connection.address.sin_addr)<<std::endl;
}

int SimpleServer::serve() {
    if(listen(file_descriptor, backlog) == -1){
        return -1;
    }

    ConnectionInfo connection = accept_connection();
    receive(connection, 1000);

    close(connection.file_descriptor);
    return 0;
}


SimpleServer::~SimpleServer (){
    // closing all the resources.
    close(file_descriptor);
}
