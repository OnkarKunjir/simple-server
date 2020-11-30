#pragma once
#include <netinet/in.h>

struct ConnectionInfo {
    int file_descriptor;
    struct sockaddr_in address;
};

class SimpleServer {
    private:
        int file_descriptor, backlog;
        struct sockaddr_in address;

    public:
        SimpleServer(const char* ip_address, int port, int backlog);

        int init();

        ConnectionInfo accept_connection();

        int receive(const ConnectionInfo &connection, int buffer_size);

        void send_message(const ConnectionInfo &connection, const char* message, int message_len);

        void print_connection_info(const ConnectionInfo &connection);

        int serve();

        ~SimpleServer();
};
