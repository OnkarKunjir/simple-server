#include <cstdio>
#include <cstring>
#include <string>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../include/server.hpp"

SimpleServer::SimpleServer(const char* ip_address, int port, int backlog) {
    // server consturctor initalizes only address structure.
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

void SimpleServer::print_connection_info(const ConnectionInfo &connection) {
    // print the information related to connection
    // prints only ip address for now.
    printf("[Connection from] %s\n", inet_ntoa(connection.address.sin_addr));
}

ConnectionInfo SimpleServer::accept_connection() {
    // function accepts connection form clients.
    struct ConnectionInfo client;
    int address_len = sizeof(client.address);
    client.file_descriptor = accept(file_descriptor, (struct sockaddr*)&client.address, (socklen_t *)&address_len);
    if(client.file_descriptor == -1)
        printf("Failed to accept connection\n");
    return client;
}

void SimpleServer::send_message(const ConnectionInfo &connection, const char* message, int message_len){
    // function to send whole message.
    send(connection.file_descriptor, message, message_len, 0);
}


int SimpleServer::process_GET(const ConnectionInfo &connection, const char* request_header, int header_len){
    // function to process get request.

    char response[] = "HTTP/1.0 200 OK\r\n"
        "Server: Simple server\r\n"
        "Content-Length: 11\r\n"
        "\r\n"
        "Hello World";
    send_message(connection, response, strlen(response));
    return 0;
}

int SimpleServer::process_request(const ConnectionInfo &connection, int buffer_size = 1024){
    // function to process request form client.
    // NOTE: default buffer size is 1KB.

    // TODO: accepts request body and process post request also.
    // Receive the header for request.
    // buffer is used to recive data over the network.
    char header[8000], buffer[buffer_size];
    char* body = NULL;

    // recv request header.

    int header_len = 0, bytes_recvd = 0;
    bool cont_recv = true;

    while(cont_recv){
        bytes_recvd  = recv(connection.file_descriptor, buffer, buffer_size, 0);
        if(bytes_recvd < 0){
            // something went wrong while reciving bytes
            printf("failed to recive packet");
            return -1;
        }
        else if(bytes_recvd == 0){
            // EOF file reached or connection is closed by client.
            return 0;
        }
        // add terminating symbol at the end of buffer.
        buffer[bytes_recvd] = 0;

        for(int i = 0; i < bytes_recvd; i++){
            header[header_len++] = buffer[i];

            if(header_len >4 && !strcmp(header+header_len-4, "\r\n\r\n")){
                cont_recv = false;
                break;
            }
        }
    }

    header[header_len] = 0;
    printf("%s", header);
    process_GET(connection, header, header_len);

    if(body != NULL){
        // delete body if the request had a body.
        delete [] body;
    }

    return 0;
}

int SimpleServer::serve() {
    if(listen(file_descriptor, backlog) == -1){
        return -1;
    }

    ConnectionInfo connection;
    while(true){
        connection = accept_connection();
        process_request(connection);
        close(connection.file_descriptor);
    }
    return 0;
}


SimpleServer::~SimpleServer (){
    // closing all the resources.
    close(file_descriptor);
}


// static functions down here
void SimpleServer::hex_dump(const char *buffer, int buffer_len){
    printf("hex dump for buffer of length %d\n", buffer_len);
    int group_size = 2, n_groups = 5;
    int current_byte = 0, current_group = 0;

    while(current_byte < buffer_len){
        if(current_byte != 0){
            if(current_byte % group_size == 0){
                printf("  ");
                current_group++;
            }
            if(current_group == n_groups){
                printf("\n");
                current_group = 0;
            }
        }
        printf("%02x", buffer[current_byte]);
        current_byte++;
    }
    printf("\n");
}
