#include <asm-generic/socket.h>
#include <iostream>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


const int PORT = 8080;

int main(){
    int server_fd = 0;
    int set_flag = 1, unset_flag = 0;
    struct sockaddr_in server_address;

    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        return -1;
    }
    if((setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &set_flag, sizeof(set_flag))) == -1){
        return -1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = 0;
    memset(&server_address.sin_zero, 0, sizeof(server_address.sin_zero));

    if(bind(server_fd, (struct sockaddr*)&server_address, sizeof(server_address)) == -1){
        return -1;
    }

    int client_fd = 0;
    struct sockaddr_in client_address;
    int sin_size = sizeof(client_address);

    if(listen(server_fd, 2) == -1){
        return -1;
    }

    if((client_fd = accept(server_fd, (struct sockaddr*)&client_address, (socklen_t*)&sin_size)) == -1){
        perror("accept");
        exit(EXIT_FAILURE);
        return -1;
    }

    char buffer[100];
    if(recv(client_fd, (void *)buffer, 100, 0) == -1){
        std::cout<<"fail\n";
        return -1;
    }
    std::cout<<buffer<<std::endl;

    return 0;
}
