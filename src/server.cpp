#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <unistd.h>
#include <iostream>
#include <fstream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../include/server.hpp"
#include "../include/utils.hpp"

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
    int bytes_sent = 0, start = 0;
    while(true){
        bytes_sent = send(connection.file_descriptor, message + start, message_len, 0);
        if(bytes_sent < 0){
            printf("Something went wrong while sending data\n");
            break;
        }
        start += bytes_sent;
        message_len -= bytes_sent;
        if(message_len == 0){
            // whole message is sent
            break;
        }
    }
}


int SimpleServer::process_GET(const ConnectionInfo &connection, const char* request_header, int header_len){
    // function to process get request.

    Header parsed_header = parse_header(request_header, header_len);

    // generate response.
    Header response_header;
    response_header.version = "HTTP/1.0";

    std::string file_path;
    if(parsed_header.resource_name == "/"){
        file_path = "./public/index.html";
        response_header.content_type = "text/html";
    }
    else{
        auto path_parts = split(parsed_header.resource_name, "/");
        auto resourse_parts = split(path_parts[path_parts.size()-1], ".");
        std::string extension = resourse_parts[resourse_parts.size()-1];
        if(extension == "jpg")
            response_header.content_type = "image/jpeg";
        else if(extension == "html")
            response_header.content_type = "text/html";

        file_path = "./public" + parsed_header.resource_name;
    }
    printf("requesting %s\n", file_path.c_str());
    std::fstream requested_file(file_path.c_str(), std::fstream::in | std::fstream::binary);


    if(requested_file.is_open()){
        response_header.status = 200;
        response_header.msg = "OK";

        requested_file.seekg(0, std::fstream::end);
        response_header.content_length = (int)requested_file.tellg();
        requested_file.seekg(0, std::fstream::beg);

        // send the head of response.
        auto response_header_str = SimpleServer::header_to_str(response_header);
        send_message(connection, response_header_str.c_str(), response_header_str.length());

        // send body of response.
        char response_body[response_header.content_length+1];
        memset(response_body, 0, response_header.content_length+1);

        while(requested_file){
            requested_file.read(response_body, response_header.content_length+1);
            send_message(connection, response_body, response_header.content_length);
        }

        requested_file.close();
    }
    else{
        char response_body[] = "404 File Not Found!";

        response_header.status = 404;
        response_header.msg = "Not Found";
        response_header.content_type = "text/html";
        response_header.content_length = strlen(response_body);
        // send the head of response.
        auto response_header_str = SimpleServer::header_to_str(response_header);
        send_message(connection, response_header_str.c_str(), response_header_str.length());

        send_message(connection, response_body, strlen(response_body));
    }

    return 0;
}

int SimpleServer::process_request(const ConnectionInfo &connection, int buffer_size = 1024){
    // function to process request form client.
    // NOTE: default buffer size is 1KB.

    // TODO: accepts request body and process post request also.
    // Receive the header for request.
    // buffer is used to recive data over the network.
    char header[8000], buffer[buffer_size];
    memset(header, 0, 8000);

    // recv request header.

    int header_len = 0, bytes_recvd = 0;
    bool cont_recv = true;

    while(cont_recv){
        memset(buffer, 0, buffer_size);
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
    process_GET(connection, header, header_len);

    return 0;
}

int SimpleServer::serve() {
    if(listen(file_descriptor, backlog) == -1){
        return -1;
    }


    while(true){
        ConnectionInfo connection = accept_connection();
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

Header SimpleServer::parse_header(std::string header, int header_len){
    auto lines = split(header, "\r\n");
    Header parsed_header;

    bool is_request_header = true;
    if(is_request_header){
        auto param = split(lines[0], " ");
        parsed_header.method = param[0];
        parsed_header.resource_name = param[1];
        parsed_header.version = param[2];
    }

    return parsed_header;
}

std::string SimpleServer::header_to_str(Header header){
    std::string header_str = "";
    bool is_request = false;
    if(!is_request){
        header_str = header.version + " " + std::to_string(header.status) + " " + header.msg + "\r\n";
    }

    if(header.content_type.length() > 0){
        header_str += "Content-Type: " + header.content_type + "\r\n";
    }
    header_str += "Content-Length: " + std::to_string(header.content_length) + "\r\n";
    header_str += "\r\n";

    return header_str;
}
